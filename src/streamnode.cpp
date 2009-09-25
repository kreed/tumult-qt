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
		if (!dynamic_cast<PlaylistStream*>(_stream))
			_stream = new PlaylistStream;
	} else if (QDir(uri).exists()) {
		if (!dynamic_cast<DirectoryStream*>(_stream))
			_stream = new DirectoryStream;
	} else {
		if (!dynamic_cast<UriStream*>(_stream))
			_stream = new UriStream;
	}

	_stream->setLocation(uri);
}

void
StreamNode::setName(const QString &name)
{
	_name = name;
}

StreamNode *
StreamNode::remove()
{
	if (_next == this)
		return NULL;

	_prev->_next = _next;
	_next->_prev = _prev;
	return _next;
}
