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

#include <phonon/mediasource.h>
#include "streamelement.h"
#include <QFile>
#include <QUrl>

StreamElement::StreamElement(const QString &name, const QString &uri)
	: _name(name)
	, _playlist(uri.endsWith("m3u"))
	, _playlistFile(_playlist ? uri : QString())
{
	if (!_playlist)
		append(uri);
}

void
StreamElement::appendUri(const QString &uri)
{
	if (uri.startsWith('/'))
		append(Phonon::MediaSource(uri));
	else
		append(Phonon::MediaSource(QUrl(uri)));
}

Phonon::MediaSource
StreamElement::source()
{
	return _playlist ? !isEmpty() || loadPlaylist()
	                 ? takeAt(qrand() % size())
	                 : Phonon::MediaSource()
	     : first();
}

void
StreamElement::setSearch(const QString &uri)
{
	_search = uri;
	_lastIndex = -1;
}

int
StreamElement::search(int i)
{
	for (int len = size(); i != len; ++i)
		if (at(i).url().path().contains(_search, Qt::CaseInsensitive))
			return i;
	return -1;
}

Phonon::MediaSource
StreamElement::nextResult()
{
	int i = search(_lastIndex + 1);
	if (i != -1) {
		_lastIndex = i;
		return at(i);
	} else if (_lastIndex != -1) {
		// loop back to the beginning
		_lastIndex = -1;
		return nextResult();
	} else
		return Phonon::MediaSource();
}

bool
StreamElement::loadPlaylist()
{
	QFile file(_playlistFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "could not open playlist" << file.fileName();
		return false;
	}

	QTextStream in(&file);
	while (!in.atEnd())
		append(in.readLine());

	return true;
}
