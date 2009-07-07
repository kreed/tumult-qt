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
	, nextInStreamAction(new QAction("Next Item in Stream", this))
	, nextInQueueAction(new QAction("Next Item in Queue", this))
	, _message(new MessageWindow)
	, _expectingSourceChange(false)
	, _metaDataInvalid(false)
	, _showNextMetaData(false)
{
	Phonon::AudioOutput *audioOutput =
		new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(this, audioOutput);

	connect(this, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)),
	              SLOT(newSource(const Phonon::MediaSource&)));
	connect(this, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
	              SLOT(newState(Phonon::State, Phonon::State)));
	connect(this, SIGNAL(aboutToFinish()),
	              SLOT(loadAnother()));

	connect(showStatusAction, SIGNAL(triggered()), SLOT(showStatus()));
	connect(searchAction, SIGNAL(triggered()), SLOT(openSearchBox()));
	connect(playPauseAction, SIGNAL(triggered()), SLOT(playPause()));
	connect(prevStreamAction, SIGNAL(triggered()), SLOT(prevStream()));
	connect(nextStreamAction, SIGNAL(triggered()), SLOT(nextStream()));
	connect(nextInStreamAction, SIGNAL(triggered()), SLOT(nextInStream()));
	connect(nextInQueueAction, SIGNAL(triggered()), SLOT(nextInQueue()));

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

	instance = this;
}

bool
Player::checkEmptyStream()
{
	if (currentStream()->count() == 0) {
		currentStream()->repopulate();
		if (currentStream()->count() == 0) {
			showStatus(false);
			return true;
		} else
			changeSource(currentStream()->source());
	}

	return false;
}

static QString
formatTime(qint64 time)
{
	int sec = qRound(time / 1000.0);
	int min = sec / 60;
	sec -= min * 60;
	return QString(QLatin1String("%1:%2")).arg(min).arg(sec, 2, 10, QLatin1Char('0'));
}

void
Player::showStatus(bool metadata)
{
	if (currentStream()->count() == 0) {
		if (currentStream()->error().isEmpty())
			_message->showText("No sources in stream");
		else
			_message->showText(currentStream()->error());
		return;
	}

	switch (state()) {
	case Phonon::ErrorState:
		_message->showText(errorString() + '\n' + currentSource().url().toString());
		break;
	case Phonon::PlayingState:
	case Phonon::LoadingState:
	case Phonon::BufferingState:
		if (metadata) {
			if (_metaDataInvalid) {
				_showNextMetaData = true;
				return;
			}

			QMultiMap<QString, QString> metadata = metaData();
			metadata.insert("STREAM", currentStream()->name());
			metadata.insert("URL", currentSource().url().toString());
			metadata.insert("PROGRESS", QString(QLatin1String("%1 / %2")).arg(formatTime(totalTime() - remainingTime()), formatTime(totalTime())));
			_message->showMetadata(metadata);
		} else
			_message->showText(currentStream()->name());
		break;
	case Phonon::PausedState:
	case Phonon::StoppedState:
		_message->showText("Not Playing");
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
	clearQueue();
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
Player::shiftStream()
{
	if (_searchBox)
		_searchBox->close();
	changeSource(currentStream()->source());
	showStatus(false);
}

void
Player::prevStream()
{
	if (_currentStream == _streams.constBegin())
		_currentStream = _streams.constEnd();
	--_currentStream;

	shiftStream();
}

void
Player::nextStream()
{
	if (++_currentStream == _streams.constEnd())
		_currentStream = _streams.constBegin();

	shiftStream();
}

void
Player::nextInStream()
{
	if (checkEmptyStream())
		return;

	if (state() == Phonon::PlayingState) {
		if (currentStream()->count() > 1)
			changeSource(currentStream()->source());
	} else
		play();
}

void
Player::nextInQueue()
{
	if (checkEmptyStream())
		return;

	if (state() == Phonon::PlayingState) {
		if (currentStream()->count() < 2)
			return;

		QList<Phonon::MediaSource> next = queue();
		if (next.isEmpty())
			return;

		_message->hide();
		setCurrentSource(next.takeAt(qrand() % next.length()));
		setQueue(next);
	}

	play();
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

	if (qApp->keyboardModifiers() == Qt::NoModifier) {
		if (currentStream()->currentSearch() != text)
			currentStream()->setSearch(text);

		const Phonon::MediaSource source = currentStream()->nextResult();
		if (source.type() != Phonon::MediaSource::Empty) {
			_showNextMetaData = true;
			changeSource(source);
		}
	} else {
		setQueue(currentStream()->allResults(text));
		_showNextMetaData = true;
		nextInQueue();
	}

	_lastSearch = text;
}

void
Player::loadAnother()
{
	enqueue(currentStream()->source());
}

void
Player::newState(Phonon::State news, Phonon::State olds)
{
	if (news == Phonon::PlayingState && olds == Phonon::LoadingState) {
		_metaDataInvalid = false;
		if (_showNextMetaData) {
			_showNextMetaData = false;
			showStatus(true);
		}
	}
	playPauseAction->setChecked(news == Phonon::PlayingState);
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
}
