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

#ifndef STREAMNODE_H
#define STREAMNODE_H

#include <qobject.h>

class Stream;

class StreamNode : public QObject {
	Q_OBJECT
public:
	StreamNode(const QString &name, const QString &uri, StreamNode *sibling);

	inline Stream *stream() const { return _stream; }
	inline QString name() const { return _name; }
	QString location() const;

	void setName(const QString &);
	void setLocation(const QString &);

	inline StreamNode *prevStream() const { return _prev; }
	inline StreamNode *nextStream() const { return _next; }
	StreamNode *remove();

signals:
	void nameChanged(StreamNode *);
	void locationChanged(StreamNode *);

private:
	Stream *_stream;

	StreamNode *_prev;
	StreamNode *_next;

	QString _name;
	QString _location;
};

#endif
