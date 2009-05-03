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
#include <qsettings.h>
#include <qurl.h>
#include "searchbox.h"
#include "streams/stream.h"

Player *player;

Player::Player()
	: _message(new MessageWindow)
	, _searchBox(NULL)
{
	Phonon::AudioOutput *audioOutput =
		new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(this, audioOutput);

	connect(this, SIGNAL(aboutToFinish()),
	              SLOT(loadAnother()));

	QSettings settings;
	settings.beginGroup("stream");
	foreach (const QString &key, settings.childKeys())
		_streams.append(Stream::create(key, settings.value(key).toString()));

	if (_streams.isEmpty()) {
		qWarning() << "No streams specified in" << settings.fileName();
		exit(EXIT_FAILURE);
	}

	_currentStream = _streams.constBegin();
	setCurrentSource(currentStream()->source());
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
			QMultiMap<QString, QString> metadata = metaData();
			metadata.insert("STREAM", currentStream()->name());
			metadata.insert("URL", currentSource().url().toString());
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
Player::changeSource(const Phonon::MediaSource &source)
{
	_message->hide();
	clearQueue();
	setCurrentSource(source);
	play();
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
	connect(_searchBox, SIGNAL(destroyed(QObject*)),
			            SLOT(searchBoxDestroyed()));
	_searchBox->search(_lastSearch);
}

void
Player::search()
{
	if (!_searchBox)
		return;

	const QString text = _searchBox->text();
	if (currentStream()->currentSearch() != text)
		currentStream()->setSearch(text);

	const Phonon::MediaSource source = currentStream()->nextResult();
	if (source.type() != Phonon::MediaSource::Empty) {
		showNextMetaData();
		changeSource(source);
	}

	_lastSearch = text;
}

void
Player::searchBoxDestroyed()
{
	_searchBox = NULL;
}

void
Player::loadAnother()
{
	enqueue(currentStream()->source());
}

void
Player::showNextMetaData()
{
	connect(this, SIGNAL(metaDataChanged()),
	              SLOT(showMetaData()));
}

void
Player::showMetaData()
{
	showStatus(true);
	disconnect(this, SIGNAL(metaDataChanged()),
	           this, SLOT(showMetaData()));
}

