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
#include <phonon/mediasource.h>
#include <QStringList>
#include <QUrl>
#include "searchbox.h"
#include "stream.h"

Player *player;

Player::Player()
	: _message(new MessageWindow)
	, _searchBox(new SearchBox)
{
	Phonon::AudioOutput *audioOutput =
		new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(this, audioOutput);

	connect(this, SIGNAL(aboutToFinish()),
	              SLOT(loadAnother()));
}

void
Player::showStatus(bool metadata)
{
	QString text;

	switch (state()) {
	case Phonon::ErrorState:
		text = errorString() + '\n' + currentSource().url().toString();
		break;
	case Phonon::PlayingState:
	case Phonon::LoadingState:
		if (currentStream()->count() == 0) {
			text = currentStream()->error();
			if (text.isEmpty())
				text = "No songs in playlist";
			break;
		}
	case Phonon::BufferingState:
		if (metadata) {
			QString title = metaData(Phonon::TitleMetaData).join(", ");
			QString artist = metaData(Phonon::ArtistMetaData).join(", ");

			if (artist.isEmpty() && title.contains(" - ")) {
				QStringList at = title.split(" - ");
				title = at[1];
				artist = at[0];
			}

			if (!title.isEmpty())
				text += "<b>" + title + "</b><br>";
			if (!artist.isEmpty())
				text += artist + "<br>";

			if (text.isEmpty())
				text += currentSource().url().toString() + "<br>";
		}

		text += currentStream()->name();
		break;
	default:
		text = "Not Playing";
	}

	_message->showMessage(text);
}

bool
Player::parse(const QByteArray &name, const QByteArray &uri)
{
	_streams.append(Stream::create(name, uri));
	return true;
}

void
Player::init()
{
	if (_streams.isEmpty()) {
		qWarning() << "No streams specified";
		exit(EXIT_FAILURE);
	}

	_currentStream = _streams.constBegin();
	setCurrentSource(currentStream()->source());
}

void
Player::changeSource(const Phonon::MediaSource &source)
{
	clearQueue();
	setCurrentSource(source);
	play();
}

void
Player::shiftStream()
{
	_searchBox->close();
	changeSource(currentStream()->source());
	showStatus(false);
}

void
Player::prev()
{
	if (_currentStream == _streams.constBegin())
		_currentStream = _streams.constEnd();
	--_currentStream;

	shiftStream();
}

void
Player::next()
{
	if (++_currentStream == _streams.constEnd())
		_currentStream = _streams.constBegin();

	shiftStream();
}

void
Player::action(Action action)
{
	switch (action) {
	case Prev:
		prev();
		return;
	case Next:
		next();
		return;
	default:
		break;
	}

	if (currentStream()->count() == 0) {
		changeSource(currentStream()->source());
		if (currentStream()->count() == 0) {
			showStatus(false);
			return;
		}
	}

	switch (action) {
	case PlaylistNext:
		if (state() == Phonon::PlayingState) {
			if (currentStream()->count() > 1)
				changeSource(currentStream()->source());
		} else
			play();
		break;
	case PlayPause:
		if (state() == Phonon::PlayingState) {
			if (currentSource().type() == Phonon::MediaSource::LocalFile)
				pause();
			else
				stop();
		} else
			play();
		break;
	case Search:
		if (currentStream()->count() > 1)
			_searchBox->search(&*currentStream());
		break;
	case ShowStatus:
		showStatus(true);
		break;
	default:
		break;
	}
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
