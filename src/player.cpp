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

#include "messagewindow.h"
#include <phonon/audiooutput.h>
#include <qaction.h>
#include <qapplication.h>
#include <qsettings.h>
#include <qurl.h>
#include "searchbox.h"
#include "streams/stream.h"
#include "tumult.h"

Player *Player::instance;

Player::Player()
	: showStatusAction(new QAction("Show Status", this))
	, searchAction(new QAction("Search in Stream", this))
	, playPauseAction(new QAction("Playing", this))
	, prevStreamAction(new QAction("Previous Stream", this))
	, nextStreamAction(new QAction("Next Stream", this))
	, prevInStreamAction(new QAction("Previous Item in Stream", this))
	, nextInStreamAction(new QAction("Next Item in Stream", this))
	, clearQueueAction(new QAction("Clear Queue", this))
	, _message(new MessageWindow)
	, _expectingSourceChange(false)
	, _metaDataInvalid(false)
	, _showNextMetaData(false)
	, _toSeek(0)
{
	Phonon::AudioOutput *audioOutput =
		new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(this, audioOutput);

	connect(this, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)),
	              SLOT(newSource(const Phonon::MediaSource&)));
	connect(this, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
	              SLOT(newState(Phonon::State, Phonon::State)));
	connect(this, SIGNAL(metaDataChanged()),
	              SLOT(setMetaData()));
	connect(this, SIGNAL(aboutToFinish()),
	              SLOT(loadAnother()));

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
		_streams.append(Stream::create(key, settings.value(key).toString()));

	if (_streams.isEmpty()) {
		qWarning() << "No streams specified in" << settings.fileName();
		exit(EXIT_FAILURE);
	}

	_currentStream = _streams.constBegin();
	_message->setStream(currentStream()->name());

	instance = this;
}

bool
Player::checkEmptyStream()
{
	if (currentStream()->count() == 0) {
		currentStream()->repopulate();
		if (currentStream()->count() == 0) {
			showStatus();
			return true;
		} else
			changeSource(currentStream()->source());
	}

	return false;
}

void
Player::showStatus()
{
	if (currentStream()->count() == 0) {
		if (currentStream()->error().isEmpty())
			_message->showText("No sources in stream");
		else
			_message->showText(currentStream()->error());
		return;
	}

	if (_metaDataInvalid) {
		_showNextMetaData = true;
		return;
	}

	switch (state()) {
	case Phonon::ErrorState:
		_message->showText(errorString() + '\n' + currentSource().url().toString());
		break;
	case Phonon::BufferingState:
		_message->showText("Buffering...");
		break;
	case Phonon::PlayingState:
		_message->setProgress(totalTime() - remainingTime());
		_message->showMetaData();
		break;
	case Phonon::LoadingState:
		if (currentSource().type() != Phonon::MediaSource::Empty) {
			_message->showText("Loading...");
			break;
		}
		// else we haven't loaded anything yet since the stream has just been created, don't show "Loading..."
	case Phonon::PausedState:
	case Phonon::StoppedState:
		_message->showText("Not Playing");
		break;
	}
}

void
Player::setCurrentSource(const Phonon::MediaSource &source)
{
	_expectingSourceChange = true;
	MediaObject::setCurrentSource(source);
}

void
Player::changeSource(const Phonon::MediaSource &source)
{
	_message->hide();
	Phonon::MediaObject::clearQueue();
	setCurrentSource(source);
	play();
}

void
Player::play()
{
	if (currentSource().type() == Phonon::MediaSource::Empty && !checkEmptyStream())
		setCurrentSource(currentStream()->source());
	MediaObject::play();
}

void
Player::setStream(const StreamList::const_iterator &stream)
{
	if (_searchBox)
		_searchBox->close();

	// isSeekable does not seem to actually return the correct value... so just ignore urls
	if (isSeekable() && currentSource().type() != Phonon::MediaSource::Url)
		currentStream()->setCurrentTime(currentTime());

	_currentStream = stream;
	changeSource(currentStream()->source());

	_toSeek = currentStream()->currentTime();
	_message->setStream(currentStream()->name());
}

void
Player::prevStream()
{
	StreamList::const_iterator stream = _currentStream;
	if (_currentStream == _streams.constBegin())
		stream = _streams.constEnd();
	setStream(stream - 1);
}

void
Player::nextStream()
{
	StreamList::const_iterator stream = _currentStream + 1;
	if (stream == _streams.constEnd())
		stream = _streams.constBegin();
	setStream(stream);
}

void
Player::prevInStream()
{
	if (checkEmptyStream())
		return;

	if (state() == Phonon::PlayingState) {
		if (currentStream()->prev())
			changeSource(currentStream()->source());
	} else
		play();
}

void
Player::nextInStream()
{
	if (checkEmptyStream())
		return;

	if (state() == Phonon::PlayingState) {
		if (currentStream()->next())
			changeSource(currentStream()->source());
	} else
		play();
}

void
Player::clearQueue()
{
	currentStream()->clearQueue();
}

void
Player::smartStop()
{
	if (currentSource().type() == Phonon::MediaSource::LocalFile)
		pause();
	else
		stop();
}

void
Player::playPause()
{
	if (checkEmptyStream())
		return;

	if (state() == Phonon::PlayingState)
		smartStop();
	else
		play();
}

void
Player::openSearchBox()
{
	if (_searchBox) {
		_searchBox->close();
		return;
	}

	if (checkEmptyStream() || currentStream()->count() == 1)
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

	if (text != _lastSearch || !currentStream()->hasQueue()) {
		currentStream()->fillQueue(text);
		_lastSearch = text;
	}

	if (currentStream()->hasQueue())
		nextInStream();
}

void
Player::loadAnother()
{
	currentStream()->next();
	enqueue(currentStream()->source());
}

void
Player::newState(Phonon::State news, Phonon::State olds)
{
	if (news == Phonon::PlayingState && olds == Phonon::LoadingState) {
		if (_toSeek)
			seek(_toSeek);
		_metaDataInvalid = false;
		_message->setMetaData(metaData());
		_message->setProgress(0, totalTime());
		if (_showNextMetaData) {
			_showNextMetaData = false;
			showStatus();
		}
	}
	playPauseAction->setChecked(news == Phonon::PlayingState);
}

void
Player::setMetaData()
{
	_metaDataInvalid = false;
	_message->setMetaData(metaData());
}

void
Player::saveHit()
{
	if (static_cast<Tumult*>(qApp)->idleTime() > 180000)
		return;
	QSettings settings;
	settings.beginGroup("hits");
	settings.setValue(_savedUrl, settings.value(_savedUrl, 0).toInt() + 1);
}

void
Player::newSource(const Phonon::MediaSource &src)
{
	if (_expectingSourceChange)
		_expectingSourceChange = false;
	else
		saveHit();
	_savedUrl = src.url().toString();
	_metaDataInvalid = true;
	_message->setUrl(_savedUrl);
}

void
Player::repopulateStream()
{
	currentStream()->repopulateLater();
}
