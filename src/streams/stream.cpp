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

#include "stream.h"

#include "directory.h"
#include "playlist.h"
#include <qdir.h>
#include "uri.h"

Stream::Stream(const QString &name, Stream *sibling)
	: _source(NULL)
	, _name(name)
	, _currentTime(0)
{
	_prev = sibling;
	if (sibling) {
		_next = sibling->_next ? sibling->_next : sibling;
		sibling->_next = _next->_prev = this;
	} else
		_next = NULL;
}

Stream*
Stream::create(const QString &name, const QString &uri, Stream *sibling)
{
	if (QDir(uri).exists())
		return new DirectoryStream(name, uri, sibling);
	else if (uri.endsWith(QLatin1String("m3u")))
		return new PlaylistStream(name, uri, sibling);
	else
		return new UriStream(name, uri, sibling);
}

MediaSource *
Stream::source()
{
	if (!_source)
		next();
	return _source;
}

void
Stream::fillQueue(const QString&)
{
}

bool
Stream::hasQueue() const
{
	return false;
}

void
Stream::clearQueue()
{
}

bool
Stream::prev()
{
	return false;
}

bool
Stream::next()
{
	return false;
}

void
Stream::repopulate()
{
}

void
Stream::repopulateLater()
{
}

void
Stream::setCurrentTime(qint64 time)
{
	_currentTime = time;
}
