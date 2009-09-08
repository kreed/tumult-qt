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

	QMetaObject::invokeMethod(this, "repopulateLater", Qt::QueuedConnection);
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

	_queue.prepend(_source.url().toString());
	_source = _prevSources.pop();
	return true;
}

bool
ListStream::next()
{
	if (_list.isEmpty())
		return false;

	if (_source.type() != Phonon::MediaSource::Empty) {
		_prevSources.push(_source);
		while (_prevSources.size() > 20)
			_prevSources.pop_front();
	}

	if (_queue.isEmpty())
		_source = createSource(_list.at(qrand() % _list.size()));
	else
		_source = createSource(_queue.takeFirst());
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
ListStream::repopulate()
{
	_list.clear();
	_error.clear();
	_source = Phonon::MediaSource();
	populate();
}

void
ListStream::repopulateLater()
{
	QtConcurrent::run(this, &ListStream::repopulate);
}

int
ListStream::count() const
{
	return _list.count();
}
