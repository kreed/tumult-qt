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

Player *player;

Player::Player()
{
	Phonon::AudioOutput *audioOutput =
		new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(this, audioOutput);

	_message = new QLabel;
	_message->setWindowFlags(Qt::ToolTip);
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

	connect(this, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
	        this, SLOT(onStateChange(Phonon::State, Phonon::State)));
}

void
Player::showStatus(bool metadata)
{
	switch (state()) {
	case Phonon::PlayingState: {
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
		}

		text += _currentStream->first;

		_message->setText(text);
		break;
	}
	case Phonon::ErrorState:
		_message->setText(errorString());
		break;
	default:
		_message->setText("Not Playing");
	}

	_message->adjustSize();
	_message->move(QApplication::desktop()->width() - _message->width() - 5, 5);
	_message->show();

	_hideMessage->start();
}

bool
Player::parse(const QByteArray &name, const QByteArray &uri)
{
	_streams.append(qMakePair(name, uri));
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
Player::play()
{
	if (_currentStream == 0)
		_currentStream = _streams.constBegin();

	setCurrentSource(QUrl(_currentStream->second));
	Phonon::MediaObject::play();
}

void
Player::prev()
{
	if (_currentStream == 0 || _currentStream == _streams.constBegin())
		_currentStream = _streams.constEnd();
	--_currentStream;

	play();
}

void
Player::next()
{
	if (_currentStream == 0 || ++_currentStream == _streams.constEnd())
		_currentStream = _streams.constBegin();

	play();
}

void
Player::action(Action action)
{
	switch (action) {
	case ShowStatus:
		showStatus();
		break;
	case PlayPause:
		if (state() == Phonon::PlayingState)
			stop();
		else
			play();
		break;
	case Prev:
		prev();
		break;
	case Next:
		next();
		break;
	default:
		break;
	}
}

void
Player::onStateChange(Phonon::State, Phonon::State)
{
	showStatus(false);
/*	qWarning() << next << prev;
	if (next == Phonon::ErrorState && prev == Phonon::PlayingState) {
//		QTimer::singleShot(500, this, SLOT(play()));
		stop();
		play();
	}*/
}
