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

#include "streamnode.h"

#include "directory.h"
#include "playlist.h"
#include <qdir.h>
#include "uri.h"

StreamNode::StreamNode(const QString &name, const QString &uri, StreamNode *sibling)
	: _stream(NULL)
	, _name(name)
{
	if (sibling) {
		_prev = sibling;
		_next = sibling->_next;
		sibling->_next = _next->_prev = this;
	} else {
		_next = _prev = this;
	}

	setLocation(uri);
}

StreamNode::~StreamNode()
{
	_prev->_next = _next;
	_next->_prev = _prev;
}

QString
StreamNode::location() const
{
	if (!_stream)
		return QString();

	return _stream->location();
}

void
StreamNode::setLocation(const QString &uri)
{
	if (_stream && _stream->location() == uri)
		return;

	if (uri.endsWith(QLatin1String("m3u"))) {
		if (!dynamic_cast<PlaylistStream*>(_stream)) {
			delete _stream;
			_stream = new PlaylistStream;
		}
	} else if (!uri.isEmpty() && QDir(uri).exists()) {
		if (!dynamic_cast<DirectoryStream*>(_stream)) {
			delete _stream;
			_stream = new DirectoryStream;
		}
	} else {
		if (!dynamic_cast<UriStream*>(_stream)) {
			delete _stream;
			_stream = new UriStream;
		}
	}

	_stream->setLocation(uri);
}

void
StreamNode::setName(const QString &name)
{
	if (name == _name)
		return;

	_name = name;
	emit nameChanged(this);
}

void
StreamNode::link(StreamNode *a, StreamNode *b)
{
	a->_next = b;
	b->_prev = a;
}

void
StreamNode::swap(StreamNode *a, StreamNode *b)
{
	StreamNode *beforeA = b->_prev;
	StreamNode *afterA = b->_next;
	StreamNode *beforeB = a->_prev;
	StreamNode *afterB = a->_next;

	if (beforeA == a)
		beforeA = b;
	if (afterA == a)
		afterA = b;
	if (beforeB == b)
		beforeB = a;
	if (afterB == b)
		afterB = a;

	link(beforeA, a);
	link(a, afterA);
	link(beforeB, b);
	link(b, afterB);
}
