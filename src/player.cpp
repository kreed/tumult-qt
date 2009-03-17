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

bool
Player::checkEmptyStream()
{
	if (currentStream()->count() == 0) {
		changeSource(currentStream()->source());
		if (currentStream()->count() == 0) {
			showStatus(false);
			return true;
		}
	}

	return false;
}

void
Player::showStatus(bool metadata)
{
	switch (state()) {
	case Phonon::ErrorState:
		_message->showText(errorString() + '\n' + currentSource().url().toString());
		break;
	case Phonon::PlayingState:
	case Phonon::LoadingState:
		if (currentStream()->count() == 0) {
			if (currentStream()->error().isEmpty())
				_message->showText("No sources in stream");
			else
				_message->showText(currentStream()->error());
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

			// more detailed information on duplicate request
			if (_message->verboseReady()) {
				// if Phonon gains support for it, add cover art here
				_message->showText(QString(
					"<b>Title: </b>%1<br>"
					"<b>Artist: </b>%2<br>"
					"<b>Album: </b>%3<br>"
					"<b>Date: </b>%4<br>"
					"<b>Genre: </b>%5<br>"
					"<b>Stream: </b>%6")
					.arg(title, artist
					   , metaData(Phonon::AlbumMetaData).join(", ")
					   , metaData(Phonon::DateMetaData).join(", ")
					   , metaData(Phonon::GenreMetaData).join(", ")
					   , currentStream()->name())
					, MessageWindow::MetaDataVerbose);
			} else {
				QString text;
				if (!title.isEmpty())
					text += "<b>" + title + "</b><br>";
				if (!artist.isEmpty())
					text += artist + "<br>";
				if (text.isEmpty())
					text = currentSource().url().toString() + "<br>";
				text += currentStream()->name();
				_message->showText(text, MessageWindow::MetaDataBrief);
			}
		} else
			_message->showText(currentStream()->name());
		break;
	default:
		_message->showText("Not Playing");
	}
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
	_message->clearVerboseFlag();
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
Player::playPause()
{
	if (checkEmptyStream())
		return;

	if (state() == Phonon::PlayingState) {
		if (currentSource().type() == Phonon::MediaSource::LocalFile)
			pause();
		else
			stop();
	} else
		play();
}

void
Player::openSearchBox()
{
	if (checkEmptyStream())
		return;

	if (currentStream()->count() > 1)
		_searchBox->search(&*currentStream());
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
