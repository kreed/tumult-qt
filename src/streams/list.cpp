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

#include <qfilesystemwatcher.h>
#include <qtconcurrentrun.h>
#include <qurl.h>

ListStream::ListStream()
	: _watcher(NULL)
{
}

ListStream::~ListStream()
{
	foreach (MediaSource *source, _prevSources)
		MediaBackend::deleteSource(source);
}

void
ListStream::setLocation(const QString &uri)
{
	if (!_watcher) {
		_watcher = new QFileSystemWatcher(this);
		connect(_watcher, SIGNAL(directoryChanged(const QString&)),
		                  SLOT(repopulate()));
		connect(_watcher, SIGNAL(fileChanged(const QString&)),
		                  SLOT(repopulate()));
	} else
		_watcher->removePath(_location);
	_watcher->addPath(uri);
	_location = uri;
	repopulate();
}

template<class T> static void
shuffle(QList<T> *list)
{
	int size, i;
	for (size = i = list->count(); --i != -1; )
		list->swap(i, i + qrand() % (size - i));
}

bool
ListStream::prev()
{
	if (_prevSources.isEmpty())
		return false;

	_queue.prepend(MediaBackend::sourceUrl(_source));
	_source = _prevSources.takeFirst();
	return true;
}

bool
ListStream::next()
{
	if (_list.isEmpty())
		return false;

	if (_source) {
		_prevSources.prepend(_source);
		if (_prevSources.size() > 20)
			MediaBackend::deleteSource(_prevSources.takeLast());
	}

	QString string = _queue.isEmpty() ? _list.at(qrand() % _list.size()) : _queue.takeFirst();
	_source = MediaBackend::createSource(string);
	return true;
}

void
ListStream::fillQueue(const QString &text)
{
	_queue = _list.filter(text, Qt::CaseInsensitive);
	shuffle(&_queue);
}

bool
ListStream::hasQueue() const
{
	return !_queue.isEmpty();
}

void
ListStream::clearQueue()
{
	_queue.clear();
}

void
ListStream::repopulateCall()
{
	_list.clear();
	_error.clear();
	_source = NULL;
	populate();
}

void
ListStream::repopulate()
{
	QtConcurrent::run(this, &ListStream::repopulateCall);
}

int
ListStream::count() const
{
	return _list.count();
}
