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
#include "statistics.h"
#include "stream.h"
#include "streamnode.h"
#include "tumult.h"

Player *Player::instance;

Player::Player()
	: showStatusAction(new QAction("Show Status", this))
	, searchAction(new QAction("Add to Queue...", this))
	, playPauseAction(new QAction(this))
	, prevStreamAction(new QAction("Previous Stream", this))
	, nextStreamAction(new QAction("Next Stream", this))
	, prevInStreamAction(new QAction("Previous Item in Stream", this))
	, nextInStreamAction(new QAction("Next Item in Stream", this))
	, clearQueueAction(new QAction("Clear Queue", this))
	, _backend(new MediaBackend(this))
	, _message(new MessageWindow)
	, _firstStream(NULL)
	, _currentStream(NULL)
	, _showNextMetaData(false)
	, _toSeek(0)
	, _playStartTime(QDateTime::currentDateTime())
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
	                  SLOT(updatePlaying(bool)));
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

	updatePlaying(false);

	QSettings settings;

	StreamNode *lastStream = NULL;
	QString lastStreamName = settings.value("streams/active").toString();

	int count = settings.beginReadArray("streams");
	for (int i = 0; i != count; ++i) {
		settings.setArrayIndex(i);
		QString name = settings.value("name").toString();
		QString location = settings.value("location").toString();
		_firstStream = new StreamNode(name, location, _firstStream);
		if (name == lastStreamName)
			lastStream = _firstStream;
	}
	settings.endArray();

	if (_firstStream) {
		_firstStream = _firstStream->nextStream();
		setStream(lastStream ? lastStream : _firstStream, false);
	}

	instance = this;
}

bool
Player::fixNoStream()
{
	if (!_currentStream) {
		showStatus();
		return false;
	}

	return true;
}

bool
Player::fixEmptyOrStopped()
{
	if (!fixNoStream())
		return false;

	if (_currentStream->stream()->count() == 0) {
		_currentStream->stream()->repopulate();
		return false;
	}

	if (!_backend->isPlaying() && _backend->errorString().isEmpty()) {
		_backend->play(_backend->isSourceNull() ? _currentStream->stream()->source() : 0);
		return false;
	}

	return true;
}

void
Player::showStatus()
{
	if (!_currentStream) {
		_message->showText("No streams specified. Add some in the settings dialog.");
		return;
	}

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
	connect(stream, SIGNAL(destroyed(QObject *)),
	                SLOT(streamDestroyed(QObject *)));

	_toSeek = _currentStream->stream()->currentTime();
	_message->setStream(_currentStream->name());
	searchAction->setEnabled(_currentStream->stream()->count() != 1);

	emit streamChanged(old, stream);

	_saveTimer.start(3000, this);
}

void
Player::prevStream()
{
	if (!fixNoStream())
		return;

	setStream(_currentStream->prevStream());
}

void
Player::nextStream()
{
	if (!fixNoStream())
		return;

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
	if (!fixNoStream())
		return;

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
	if (!fixNoStream())
		return;

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
	Statistics::sourcePlayed(url);
}

void
Player::repopulateStream()
{
	if (_currentStream)
		_currentStream->stream()->repopulate();
}

void
Player::newStreamName(StreamNode *stream)
{
	if (stream == _currentStream)
		_message->setStream(_currentStream->name());
}

void
Player::streamDestroyed(QObject *object)
{
	if (object == _currentStream)
		setStream(_currentStream->nextStream(), _backend->isPlaying());
}

StreamNode *
Player::streamAt(int i) const
{
	StreamNode *stream = _firstStream;
	if (!stream)
		return NULL;

	for (int j = 0; j != i; ++j) {
		stream = stream->nextStream();
		if (stream == _firstStream)
			return NULL;
	}

	return stream;
}

int
Player::indexOf(StreamNode *needle) const
{
	StreamNode *stream = _firstStream;
	if (!stream)
		return -1;

	for (int i = 0; ; ++i) {
		if (stream == needle)
			return i;
		stream = stream->nextStream();
		if (stream == _firstStream)
			break;
	}

	return -1;
}

int
Player::streamCount() const
{
	StreamNode *stream = _firstStream;
	if (!stream)
		return 0;

	int count = 1;

	for (; stream->nextStream() != _firstStream; ++count)
		stream = stream->nextStream();

	return count;
}

void
Player::save() const
{
	if (!_firstStream)
		return;

	if (_backend->isPlaying()) {
		QDateTime now = QDateTime::currentDateTime();
		Statistics::addPlayTime(_playStartTime.secsTo(now));
		_playStartTime = now;
	}

	QSettings settings;
	settings.setValue("streams/active", _currentStream->name());
	settings.beginWriteArray("streams", streamCount());

	StreamNode *stream = _firstStream;
	for (int i = 0; ; ++i) {
		settings.setArrayIndex(i);
		settings.setValue("name", stream->name());
		settings.setValue("location", stream->location());
		stream = stream->nextStream();
		if (stream == _firstStream)
			break;
	}

	settings.endArray();
}

void
Player::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _saveTimer.timerId()) {
		save();
		_saveTimer.stop();
	}
}

void
Player::updatePlaying(bool playing)
{
	playPauseAction->setText(playing ? "Pause" : "Play");
	if (playing)
		_playStartTime = QDateTime::currentDateTime();
	else
		Statistics::addPlayTime(_playStartTime.secsTo(QDateTime::currentDateTime()));
}
