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

class ListStream : protected QObject, public Stream {
	Q_OBJECT
public:
	ListStream();
	~ListStream();

	int count() const;

	void setLocation(const QString &);

	bool prev();
	bool next();

	void fillQueue(const QString &search);
	bool hasQueue() const;
	void clearQueue();

public slots:
	void repopulate();

protected:
	virtual void populate() = 0;

	QStringList _list;

private:
	void repopulateCall();

	QStringList _queue;
	QFileSystemWatcher *_watcher;
	QList<MediaSource*> _prevSources;
};

#endif
