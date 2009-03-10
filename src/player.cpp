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

#include <phonon/audiooutput.h>
#include <phonon/mediasource.h>
#include "player.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include "searchbox.h"
#include "streamelement.h"

Player *player;

Player::Player()
	: _searchBox(new SearchBox)
{
	Phonon::AudioOutput *audioOutput =
		new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(this, audioOutput);

	_message = new QLabel;
	_message->setWindowFlags(Qt::X11BypassWindowManagerHint);
	_message->setAlignment(Qt::AlignCenter);
	_message->setMargin(5);

	QPalette p = _message->palette();
	p.setColor(QPalette::Window, Qt::black);
	p.setColor(QPalette::WindowText, Qt::white);
	_message->setPalette(p);

	_hideMessage = new QTimer(this);
	_hideMessage->setInterval(3500);
	_hideMessage->setSingleShot(true);
	connect(_hideMessage, SIGNAL(timeout()),
	        _message, SLOT(hide()));

	connect(this, SIGNAL(aboutToFinish()),
	              SLOT(loadAnother()));
}

void
Player::showStatus(bool metadata)
{
	switch (state()) {
	case Phonon::ErrorState:
		_message->setText(errorString());
		break;
	case Phonon::PlayingState:
	case Phonon::BufferingState:
	case Phonon::LoadingState:
		if (_currentStream != StreamList::iterator()) {
			QString text;

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

			text += _currentStream->name();

			_message->setText(text);
			break;
		}
	default:
		_message->setText("Not Playing");
	}

	_message->adjustSize();
	_message->move(QApplication::desktop()->width() - _message->width() - 5, 5);
	_message->show();
	_message->raise();

	_hideMessage->start();
}

bool
Player::parse(const QByteArray &name, const QByteArray &uri)
{
	_streams.append(StreamElement(name, uri));
	return true;
}

void
Player::init()
{
	if (_streams.isEmpty()) {
		qWarning() << "No streams specified";
		exit(EXIT_FAILURE);
	}

	_currentStream = 0;
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
	changeSource(_currentStream->source());
	showStatus(false);
}

void
Player::prev()
{
	if (_currentStream == StreamList::iterator() || _currentStream == _streams.constBegin())
		_currentStream = _streams.end();
	--_currentStream;

	shiftStream();
}

void
Player::next()
{
	if (_currentStream == StreamList::iterator() || ++_currentStream == _streams.constEnd())
		_currentStream = _streams.begin();

	shiftStream();
}

void
Player::action(Action action)
{
	switch (action) {
	case PlaylistNext:
		if (state() == Phonon::PlayingState)  {
			if (_currentStream->isPlaylist())
				changeSource(_currentStream->source());
			break;
		}
	case PlayPause:
		if (state() == Phonon::PlayingState) {
			if (currentSource().type() == Phonon::MediaSource::LocalFile)
				pause();
			else
				stop();
		} else if (_currentStream == StreamList::iterator())
			next();
		else
			play();
		break;
	case Prev:
		prev();
		break;
	case Next:
		next();
		break;
	case Search:
		if (_currentStream != StreamList::iterator() && _currentStream->isPlaylist())
			_searchBox->search(&*_currentStream);
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
	enqueue(_currentStream->source());
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
