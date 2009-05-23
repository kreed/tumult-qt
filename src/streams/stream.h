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

#ifndef STREAM_H
#define STREAM_H

#include <qstring.h>

namespace Phonon {
class MediaSource;
}

class Stream {
public:
	static Stream *create(const QString &name, const QString &uri);

	virtual Phonon::MediaSource source() const = 0;

	virtual void repopulate();

	inline QString currentSearch() const { return _search; }
	void setSearch(const QString &search);
	virtual Phonon::MediaSource nextResult();
	virtual QList<Phonon::MediaSource> allResults(const QString &search) const;

	virtual int count() const = 0;

	inline QString name() const { return _name; }
	inline QString error() const { return _error; }

	static Phonon::MediaSource createSource(const QString &source);

protected:
	Stream(const QString &name);

	QString _error;

	QString _search;
	int _lastIndex;

private:
	QString _name;
};

#endif
