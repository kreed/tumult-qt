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

#ifndef MEDIABACKEND_PHONON_P
#define MEDIABACKEND_PHONON_P

#include <phonon/mediaobject.h>

class MediaBackend;

class MediaBackendPrivate : public Phonon::MediaObject {
	Q_OBJECT
private slots:
	void newState(Phonon::State news, Phonon::State olds);
	void newSource(const Phonon::MediaSource &src);
	void newMetaData();

private:
	MediaBackendPrivate(MediaBackend *parent);

	MediaBackend *q;
	bool _expectingSourceChange;
	bool _metaDataInvalid;
	QString _savedUrl;

	friend class MediaBackend;
};

#endif
