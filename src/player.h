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

#ifndef PLAYER_H
#define PLAYER_H

#include <phonon/mediaobject.h>
#include <QLinkedList>

class QLabel;
class QTimer;
typedef QLinkedList<QPair<QByteArray, QByteArray> > StreamList;

class Player : public Phonon::MediaObject {
	Q_OBJECT
public:
	enum Action {
		Next,
		Prev,
		ShowStatus,
		PlayPause,
		ActionCount
	};

	Player();
	bool parse(const QByteArray &name, const QByteArray &uri);
	void action(Action);
	void init();

	void showStatus(bool metadata = true);
	void next();
	void prev();
	void play();

protected slots:
	void onStateChange(Phonon::State, Phonon::State);

private:
	StreamList _streams;
	StreamList::const_iterator _currentStream;
	QLabel *_message;
	QTimer *_hideMessage;
};

extern Player *player;

#endif
