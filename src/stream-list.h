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

#ifndef STREAMPLAYLIST_H
#define STREAMPLAYLIST_H

#include <phonon/mediasource.h>
#include <QList>
#include <QObject>
#include "stream.h"

class QTimerEvent;

class ListStream : QObject, public Stream, QList<Phonon::MediaSource> {
	Q_OBJECT
public:
	ListStream(const QString &name, const QString &uri);

	Phonon::MediaSource source();
	Phonon::MediaSource nextResult();
	int count() const;

protected:
	void timerEvent(QTimerEvent*);

private:
	void appendUri(const QString &uri);
	int search(int from);
	void loadPlaylist();

	QString _playlistFile;
	unsigned _modTime;
};

#endif