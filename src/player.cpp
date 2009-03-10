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
	              SLOT(loadMore()));
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
		if (_currentStream != 0) {
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

			text += (*_currentStream)->name();

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
	_streams.append(new StreamElement(name, uri));
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
Player::insertUri(const QString &uri, bool now)
{
	if (now) {
		if (uri.startsWith('/'))
			setCurrentSource(uri);
		else
			setCurrentSource(QUrl(uri));
		play();
	} else {
		if (uri.startsWith('/'))
			enqueue(uri);
		else
			enqueue(QUrl(uri));
	}
}

void
Player::shiftStream()
{
	_searchBox->close();
	insertUri((*_currentStream)->uri());
	showStatus(false);
}

void
Player::prev()
{
	if (_currentStream == 0 || _currentStream == _streams.constBegin())
		_currentStream = _streams.constEnd();
	--_currentStream;

	shiftStream();
}

void
Player::next()
{
	if (_currentStream == 0 || ++_currentStream == _streams.constEnd())
		_currentStream = _streams.constBegin();

	shiftStream();
}

void
Player::action(Action action)
{
	switch (action) {
	case ShowStatus:
		showStatus(true);
		break;
	case PlayPause:
		if (state() == Phonon::PlayingState) {
			if (currentSource().url().scheme() == "file")
				pause();
			else
				stop();
		} else if (_currentStream == 0)
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
		if (_currentStream != 0 && (*_currentStream)->isPlaylist())
			_searchBox->search(*_currentStream);
		break;
	case PlaylistNext:
		if (state() != Phonon::PlayingState)
			play();
		else if ((*_currentStream)->isPlaylist())
			insertUri((*_currentStream)->uri());
		break;
	default:
		break;
	}
}

void
Player::loadMore()
{
	insertUri((*_currentStream)->uri(), false);
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
