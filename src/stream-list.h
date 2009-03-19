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

#ifndef STREAMLIST_H
#define STREAMLIST_H

#include <qstringlist.h>
#include <qobject.h>
#include "stream.h"

class QFileSystemWatcher;

class ListStream : protected QObject, public Stream, protected QStringList {
	Q_OBJECT
public:
	Phonon::MediaSource source();
	Phonon::MediaSource nextResult();
	int count() const;

	QString listSrc() const { return _listSrc; }

public slots:
	void repopulate();
	void repopulateLater();

protected:
	ListStream(const QString &name, const QString &source);

	virtual void populate();

private:
	Phonon::MediaSource createSource(const QString &uri);
	int search(int from);

	QString _listSrc;
	QFileSystemWatcher *_watcher;
};

#endif
