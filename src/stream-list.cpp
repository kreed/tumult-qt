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

#include "stream-list.h"

#include <phonon/mediasource.h>
#include <QDateTime>
#include <QFileInfo>
#include <QTimerEvent>
#include <QUrl>

ListStream::ListStream(const QString &name, const QString &uri)
	: Stream(name)
	, _listSrc(uri)
{
	startTimer(0);
}

Phonon::MediaSource
ListStream::createSource(const QString &uri)
{
	if (uri.startsWith('/'))
		return Phonon::MediaSource(uri);
	else
		return Phonon::MediaSource(QUrl(uri));
}

bool
ListStream::isListSrcModified() const
{
	return QFileInfo(_listSrc).lastModified().toTime_t() != _modTime;
}

Phonon::MediaSource
ListStream::source()
{
	if (isEmpty())
		repopulate();
	else if (size() == 1 || isListSrcModified())
		startTimer(0);
	return isEmpty() ? Phonon::MediaSource() : createSource(takeAt(qrand() % size()));
}

int
ListStream::search(int i)
{
	for (int len = size(); i != len; ++i)
		if (at(i).contains(_search, Qt::CaseInsensitive))
			return i;
	return -1;
}

Phonon::MediaSource
ListStream::nextResult()
{
	int i = search(_lastIndex + 1);
	if (i != -1) {
		_lastIndex = i;
		return createSource(at(i));
	} else if (_lastIndex != -1) {
		// loop back to the beginning
		_lastIndex = -1;
		return nextResult();
	} else
		return Phonon::MediaSource();
}

void
ListStream::populate()
{
}

void
ListStream::repopulate()
{
	clear();
	_error.clear();
	populate();
	_modTime = QFileInfo(_listSrc).lastModified().toTime_t();
}

void
ListStream::timerEvent(QTimerEvent *ev)
{
	killTimer(ev->timerId());
	repopulate();
}

int
ListStream::count() const
{
	return QStringList::count();
}
