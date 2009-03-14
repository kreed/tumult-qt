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

#include "streamplaylist.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTimerEvent>
#include <QUrl>

StreamPlaylist::StreamPlaylist(const QString &name, const QString &uri)
	: Stream(name)
	, _playlistFile(uri)
{
	startTimer(0);
}

void
StreamPlaylist::appendUri(const QString &uri)
{
	if (uri.startsWith('/'))
		append(Phonon::MediaSource(uri));
	else
		append(Phonon::MediaSource(QUrl(uri)));
}

Phonon::MediaSource
StreamPlaylist::source()
{
	if (isEmpty())
		loadPlaylist();
	else if (size() == 1 || QFileInfo(_playlistFile).lastModified().toTime_t() != _modTime)
		startTimer(0);
	return isEmpty() ? Phonon::MediaSource() : takeAt(qrand() % size());
}

int
StreamPlaylist::search(int i)
{
	for (int len = size(); i != len; ++i)
		if (at(i).url().path().contains(_search, Qt::CaseInsensitive))
			return i;
	return -1;
}

Phonon::MediaSource
StreamPlaylist::nextResult()
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

void
StreamPlaylist::loadPlaylist()
{
	QFile file(_playlistFile);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		_error = QString("Could not read playlist '%1'").arg(file.fileName());
		qWarning(qPrintable(_error));
	} else {
		_error.clear();

		QTextStream in(&file);
		while (!in.atEnd())
			append(in.readLine());

		_modTime = QFileInfo(file).lastModified().toTime_t();
	}
}

void
StreamPlaylist::timerEvent(QTimerEvent *ev)
{
	killTimer(ev->timerId());
	loadPlaylist();
}

int
StreamPlaylist::count() const
{
	return QList<Phonon::MediaSource>::count();
}
