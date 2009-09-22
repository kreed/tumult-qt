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

#ifndef MEDIABACKEND_H
#define MEDIABACKEND_H

#include <qobject.h>

class MediaBackendPrivate;
class MediaSource;

class MediaBackend : public QObject {
	Q_OBJECT
public:
	enum State {
		PlayingState,
		LoadingState,
		BufferingState,
		NotPlayingState
	};
	enum MetaData {
		Title,
		Artist,
		Album,
		Date
	};

	MediaBackend(QObject *parent = NULL);

	static MediaSource *createSource(const QString &uri);
	static void deleteSource(MediaSource *);
	static QString sourceUrl(MediaSource *);

	bool isMetaDataInvalid() const;
	bool isPlaying() const;
	bool isSourceNull() const;
	bool isSeekable() const;
	QString errorString() const;
	State state() const;
	qint64 progress() const;
	qint64 duration() const;

	QString metaData(MetaData) const;

	void play(MediaSource *source = NULL);
	void pause();
	// this should only be called in response to a newSourceNeeded signal
	void push(MediaSource *source);

	void seek(qint64 position);

signals:
	// follow with a call to MediaBackend::push
	void newSourceNeeded();

	void playingChanged(bool playing);
	void metaDataChanged(MediaBackend *);

	void sourceChanged(const QString &url);
	void sourceLoaded();
	// source has played to the end
	void sourceFinished(const QString &url);

private:
	MediaBackendPrivate *d;

	friend class MediaBackendPrivate;
};

#endif
