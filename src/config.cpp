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

#include "config.h"
#include "player.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHash>

QHash<QByteArray, Callback> *Config::map = NULL;

void
Config::add(const QByteArray &key, Callback cb)
{
	if (!map)
		map = new QHash<QByteArray, Callback>;

	map->insert(key, cb);
}

void
Config::parse()
{
	QFile file(qApp->arguments().size() == 1 ? QDir::homePath() + "/.config/tumult/rc"
	                                         : qApp->arguments().last());
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "could not open config file" << file.fileName();
		return;
	}

	while (!file.atEnd()) {
		QByteArray in = file.readLine();
		in.chop(1);
		QList<QByteArray> line = in.split('\t');
		if (line.size() == 2) {
			if (map->contains(line.at(0)))
				map->value(line.at(0))(line.at(1));
			else
				player->parse(line.at(0), line.at(1));
		}
	}

	delete map;
}

