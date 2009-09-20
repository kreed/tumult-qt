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

#include "mediabackend.h"
#include <qstring.h>

class Stream {
public:
	static Stream *create(const QString &name, const QString &uri, Stream *sibling);

	MediaSource *source();

	virtual void repopulate();
	virtual void repopulateLater();

	virtual bool prev();
	virtual bool next();

	virtual void fillQueue(const QString &search);
	virtual bool hasQueue() const;
	virtual void clearQueue();

	virtual int count() const = 0;

	inline QString name() const { return _name; }
	inline QString error() const { return _error; }

	inline Stream *prevStream() const { return _prev; }
	inline Stream *nextStream() const { return _next; }

	qint64 currentTime() const { return _currentTime; }
	void setCurrentTime(qint64);

protected:
	Stream(const QString &name, Stream *stream);

	QString _error;
	MediaSource *_source;

private:
	QString _name;
	qint64 _currentTime;
	Stream *_prev;
	Stream *_next;
};

#endif
