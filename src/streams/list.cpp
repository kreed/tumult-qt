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

#include "list.h"

#include <phonon/mediasource.h>
#include <qfilesystemwatcher.h>
#include <qtconcurrentrun.h>
#include <qtimer.h>

ListStream::ListStream(const QString &name, const QString &uri)
	: Stream(name)
	, _listSrc(uri)
	, _watcher(new QFileSystemWatcher(this))
{
	_watcher->addPath(uri);
	connect(_watcher, SIGNAL(directoryChanged(const QString&)),
	                  SLOT(repopulateLater()));
	connect(_watcher, SIGNAL(fileChanged(const QString&)),
	                  SLOT(repopulateLater()));

	QTimer::singleShot(0, this, SLOT(repopulateLater()));
}

Phonon::MediaSource
ListStream::source() const
{
	return _list.isEmpty() ? Phonon::MediaSource() : createSource(_list.at(qrand() % _list.size()));
}

int
ListStream::search(const QString &search, int i) const
{
	for (int len = _list.size(); i != len; ++i)
		if (_list.at(i).contains(search, Qt::CaseInsensitive))
			return i;
	return -1;
}

Phonon::MediaSource
ListStream::nextResult()
{
	int i = search(_search, _lastIndex + 1);
	if (i != -1) {
		_lastIndex = i;
		return createSource(_list.at(i));
	} else if (_lastIndex != -1) {
		// loop back to the beginning
		_lastIndex = -1;
		return nextResult();
	} else
		return Phonon::MediaSource();
}

QList<Phonon::MediaSource>
ListStream::allResults(const QString &text) const
{
	QList<Phonon::MediaSource> result;
	int i = -1;
	while ((i = search(text, i + 1)) != -1)
		result << createSource(_list.at(i));
	return result;
}

void
ListStream::repopulate()
{
	_list.clear();
	_error.clear();
	populate();
}

static void
callRepopulate(ListStream *stream)
{
	stream->repopulate();
}

void
ListStream::repopulateLater()
{
	QtConcurrent::run(callRepopulate, this);
}

int
ListStream::count() const
{
	return _list.count();
}
