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

#include "directory.h"

#include <qdiriterator.h>

void
DirectoryStream::populate(QStringList *list)
{
	QDirIterator it(_location,
		QStringList() << "*.m4a" << "*.ogg" << "*.mp3" << "*.flac",
		QDir::Files, QDirIterator::Subdirectories);
	while (it.hasNext())
		list->append(it.next());
}
