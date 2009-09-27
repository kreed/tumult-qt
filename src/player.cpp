/*
 * Copyright © 2009 Christopher Eby <kreed@kreed.org>
 *
 * This file is part of Tumult.
 *
 * Tumult is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Tumult is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "player.h"

#include "mediabackend.h"
#include "messagewindow.h"
#include <qaction.h>
#include <qapplication.h>
#include <qsettings.h>
#include <qurl.h>
#include "searchbox.h"
#include "stream.h"
#include "streamnode.h"
#include "tumult.h"

Player *Player::instance;

Player::Player()
	: showStatusAction(new QAction("Show Status", this))
	, searchAction(new QAction("Add to Queue...", this))
	, playPauseAction(new QAction("Playing", this))
	, prevStreamAction(new QAction("Previous Stream", this))
	, nextStreamAction(new QAction("Next Stream", this))
	, prevInStreamAction(new QAction("Previous Item in Stream", this))
	, nextInStreamAction(new QAction("Next Item in Stream", this))
	, clearQueueAction(new QAction("Clear Queue", this))
	, _backend(new MediaBackend(this))
	, _message(new MessageWindow)
	, _currentStream(NULL)
	, _showNextMetaData(false)
	, _toSeek(0)
{
	connect(_backend, SIGNAL(sourceChanged(const QString &)),
	        _message, SLOT(setUrl(const QString &)));
	connect(_backend, SIGNAL(sourceFinished(const QString &)),
	                  SLOT(saveHit(const QString &)));
	connect(_backend, SIGNAL(metaDataChanged(MediaBackend *)),
	        _message, SLOT(setMetaData(MediaBackend *)));
	connect(_backend, SIGNAL(newSourceNeeded()),
	                  SLOT(loadAnother()));
	connect(_backend, SIGNAL(playingChanged(bool)),
	        playPauseAction, SLOT(setChecked(bool)));
	connect(_backend, SIGNAL(sourceLoaded()),
	                  SLOT(newSourceLoaded()));

	connect(showStatusAction, SIGNAL(triggered()), SLOT(showStatus()));
	connect(searchAction, SIGNAL(triggered()), SLOT(openSearchBox()));
	connect(playPauseAction, SIGNAL(triggered()), SLOT(playPause()));
	connect(prevStreamAction, SIGNAL(triggered()), SLOT(prevStream()));
	connect(nextStreamAction, SIGNAL(triggered()), SLOT(nextStream()));
	connect(prevInStreamAction, SIGNAL(triggered()), SLOT(prevInStream()));
	connect(nextInStreamAction, SIGNAL(triggered()), SLOT(nextInStream()));
	connect(clearQueueAction, SIGNAL(triggered()), SLOT(clearQueue()));

	playPauseAction->setCheckable(true);

	QSettings settings;
	settings.beginGroup("stream");
	foreach (const QString &key, settings.childKeys())
		_currentStream = new StreamNode(key, settings.value(key).toString(), _currentStream);

	if (_currentStream) {
		_firstStream = _currentStream->nextStream();
		setStream(_firstStream, false);
	} else {
		qWarning("No streams specified in '%s'", qPrintable(settings.fileName()));
		exit(EXIT_FAILURE);
	}

	instance = this;
}

bool
Player::fixEmptyOrStopped()
{
	if (_currentStream->stream()->count() == 0) {
		_currentStream->stream()->repopulate();
		if (_currentStream->stream()->count() == 0) {
			showStatus();
			return false;
		}
	}

	if (!_backend->isPlaying()) {
		_backend->play(_backend->isSourceNull() ? _currentStream->stream()->source() : 0);
		return false;
	}

	return true;
}

void
Player::showStatus()
{
	if (!_currentStream->stream()->error().isEmpty()) {
		_message->showText(_currentStream->stream()->error());
		return;
	}

	if (_currentStream->stream()->count() == 0) {
		_message->showText("No sources in stream");
		return;
	}

	if (!_backend->errorString().isEmpty()) {
		_message->showText(_backend->errorString(), true);
		return;
	}

	if (_backend->isMetaDataInvalid()) {
		_showNextMetaData = true;
		return;
	}

	switch (_backend->state()) {
	case MediaBackend::BufferingState:
		_message->showText("Buffering...");
		break;
	case MediaBackend::LoadingState:
		_message->showText("Loading...");
		break;
	case MediaBackend::PlayingState:
		_message->setProgress(_backend->progress());
		_message->showMetaData();
		break;
	default:
		_message->showText("Not Playing");
		break;
	}
}

void
Player::changeSource(MediaSource *source)
{
	_message->hide();
	_backend->play(source);
}

void
Player::setStream(StreamNode *stream, bool play)
{
	if (_searchBox)
		_searchBox->close();

	if (_currentStream && _backend->isSeekable())
		_currentStream->stream()->setCurrentTime(_backend->progress());

	StreamNode *old = _currentStream;
	_currentStream = stream;
	if (play)
		changeSource(_currentStream->stream()->source());

	connect(stream, SIGNAL(nameChanged(StreamNode *)),
	                SLOT(newStreamName(StreamNode *)));
	connect(stream, SIGNAL(locationChanged(StreamNode *)),
	                SLOT(newStreamLocation(StreamNode *)));
	connect(stream, SIGNAL(destroyed(QObject *)),
	                SLOT(streamDestroyed(QObject *)));

	_toSeek = _currentStream->stream()->currentTime();
	_message->setStream(_currentStream->name());
	searchAction->setEnabled(_currentStream->stream()->count() != 1);

	emit streamChanged(old, stream);
}

void
Player::prevStream()
{
	setStream(_currentStream->prevStream());
}

void
Player::nextStream()
{
	setStream(_currentStream->nextStream());
}

void
Player::prevInStream()
{
	if (!fixEmptyOrStopped())
		return;

	if (_currentStream->stream()->prev())
		changeSource(_currentStream->stream()->source());
}

void
Player::nextInStream()
{
	if (!fixEmptyOrStopped())
		return;

	if (_currentStream->stream()->next())
		changeSource(_currentStream->stream()->source());
}

void
Player::clearQueue()
{
	_currentStream->stream()->clearQueue();
}

void
Player::playPause()
{
	if (!fixEmptyOrStopped())
		return;

	_backend->pause();
}

void
Player::openSearchBox()
{
	if (_searchBox) {
		_searchBox->close();
		return;
	}

	if (_currentStream->stream()->count() < 2)
		return;

	_searchBox = new SearchBox;
	connect(_searchBox, SIGNAL(returnPressed()),
			            SLOT(search()));
	_searchBox->search(_lastSearch);
}

void
Player::search()
{
	if (!_searchBox)
		return;

	const QString text = _searchBox->text();

	if (text != _lastSearch || !_currentStream->stream()->hasQueue()) {
		_currentStream->stream()->fillQueue(text);
		_lastSearch = text;
	}

	if (_currentStream->stream()->hasQueue())
		nextInStream();
}

void
Player::loadAnother()
{
	_currentStream->stream()->next();
	_backend->push(_currentStream->stream()->source());
}

void
Player::newSourceLoaded()
{
	if (_toSeek) {
		_backend->seek(_toSeek);
		_toSeek = 0;
	}
	qint64 duration = _backend->duration();
	_message->setProgress(0, duration ? duration : -1);
	_message->setMetaData(_backend);
	if (_showNextMetaData) {
		_showNextMetaData = false;
		showStatus();
	}
}

void
Player::saveHit(const QString &url)
{
	if (static_cast<Tumult*>(qApp)->idleTime() > 180000)
		return;
	QSettings settings;
	settings.beginGroup("hits");
	settings.setValue(url, settings.value(url, 0).toInt() + 1);
}

void
Player::repopulateStream()
{
	_currentStream->stream()->repopulateLater();
}

void
Player::newStreamName(StreamNode *stream)
{
	if (stream == _currentStream)
		_message->setStream(_currentStream->name());
}

void
Player::newStreamLocation(StreamNode *stream)
{
	if (stream == _currentStream) {
		if (fixEmptyOrStopped())
			changeSource(_currentStream->stream()->source());
	}
}

void
Player::streamDestroyed(QObject *object)
{
	if (object == _firstStream)
		_firstStream = _firstStream->nextStream();
	if (object == _currentStream)
		setStream(_currentStream->nextStream(), _backend->isPlaying());
}
