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

#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include "streamelement.h"

StreamElement::StreamElement(const QString &name, const QString &uri)
	: _name(name)
	, _uri(uri)
	, _regexp(QString(), Qt::CaseInsensitive)
	, _playlist(uri.endsWith("m3u"))
{
}

QString StreamElement::uri()
{
	return _playlist ? !_songs.isEmpty() || loadPlaylist()
	                 ? _songs.takeAt(qrand() % _songs.size())
	                 : QString()
	     : _uri;
}

void StreamElement::search(const QString &uri)
{
	_regexp.setPattern(".*" + uri + ".*");
	_lastIndex = -1;
}

QString StreamElement::nextResult()
{
	if (!_regexp.isEmpty()) {
		int i = _songs.indexOf(_regexp, _lastIndex + 1);
		if (i != -1) {
			_lastIndex = i;
			return _songs.at(i);
		} else if (_lastIndex != -1) {
			_lastIndex = -1;
			return nextResult();
		}
	}
	return QString();
}

bool StreamElement::loadPlaylist()
{
	QFile file(_uri);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "could not open playlist" << file.fileName();
		return false;
	}

	QTextStream in(&file);
	while (!in.atEnd())
		_songs.append(in.readLine());

	return true;
}
