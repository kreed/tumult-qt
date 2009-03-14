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

#include "streamplaylist.h"
#include "streamuri.h"

Stream::Stream(const QString &name)
	: _name(name)
{
}

Stream*
Stream::create(const QString &name, const QString &uri)
{
	if (uri.endsWith(QLatin1String("m3u")))
		return new StreamPlaylist(name, uri);
	else
		return new StreamUri(name, uri);
}

void
Stream::setSearch(const QString &uri)
{
	_search = uri;
	_lastIndex = -1;
}

Phonon::MediaSource
Stream::source()
{
	return Phonon::MediaSource();
}

Phonon::MediaSource
Stream::nextResult()
{
	return Phonon::MediaSource();
}

int
Stream::count() const
{
	return 0;
}
