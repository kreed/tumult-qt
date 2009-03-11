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

#ifndef STREAM_H
#define STREAM_H

#include <phonon/mediasource.h>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

class QTimerEvent;

class Stream : QObject, QList<Phonon::MediaSource> {
	Q_OBJECT
public:
	Stream(const QString &name, const QString &uri);

	Phonon::MediaSource source();

	void setSearch(const QString &search);
	Phonon::MediaSource nextResult();

	inline QString name() const { return _name; }
	inline bool isPlaylist() const { return _playlist; }
	inline bool isEmpty() const { return QList<Phonon::MediaSource>::isEmpty(); }
	inline QString error() const { return _error; }

protected:
	void timerEvent(QTimerEvent*);

private:
	void appendUri(const QString &uri);
	int search(int from);
	void loadPlaylist();

	QString _error;

	QString _search;
	int _lastIndex;

	QString _name;
	bool _playlist;
	QString _playlistFile;
	QDateTime _modTime;
};

#endif
