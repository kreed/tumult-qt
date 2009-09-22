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

#ifdef USE_QTMEDIA

#include "mediabackend.h"
#include "qtmedia_p.h"

#include <qurl.h>
#include <qvariant.h>

#include <qdebug.h>

class MediaSource : public QMediaSource {
public:
	MediaSource(const QUrl &uri)
		: QMediaSource(uri)
	{
	}
};

MediaBackendPrivate::MediaBackendPrivate(MediaBackend *parent)
	: QMediaPlayer(parent)
	, q(parent)
	, _metaDataInvalid(false)
{
	connect(this, SIGNAL(stateChanged(QMediaPlayer::State)),
	              SLOT(newState(QMediaPlayer::State)));
	connect(this, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
	              SLOT(newMediaStatus(QMediaPlayer::MediaStatus)));
	connect(this, SIGNAL(metaDataChanged()),
	              SLOT(newMetaData()));
}

void
MediaBackendPrivate::newState(QMediaPlayer::State state)
{
	if (state == QMediaPlayer::PlayingState) {
		_metaDataInvalid = false;
		emit q->sourceLoaded();
	}
	emit q->playingChanged(state == QMediaPlayer::PlayingState);
}

void
MediaBackendPrivate::newMediaStatus(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::EndOfMedia) {
		emit q->sourceFinished(media().contentUri().toString());
		emit q->newSourceNeeded();
	}
}

void
MediaBackendPrivate::newMetaData()
{
	emit q->metaDataChanged(q);
}

MediaBackend::MediaBackend(QObject *parent)
	: QObject(parent)
	, d(new MediaBackendPrivate(this))
{
}

bool
MediaBackend::isMetaDataInvalid() const
{
	return d->_metaDataInvalid;
}

bool
MediaBackend::isPlaying() const
{
	return d->state() == QMediaPlayer::PlayingState;
}

bool
MediaBackend::isSourceNull() const
{
	return d->media().isNull();
}

QString
MediaBackend::errorString() const
{
	return d->errorString();
}

MediaBackend::State
MediaBackend::state() const
{
	if (isPlaying()) {
		switch (d->mediaStatus()) {
		case QMediaPlayer::LoadingMedia:
			return MediaBackend::LoadingState;
		case QMediaPlayer::BufferingMedia:
			return MediaBackend::BufferingState;
		case QMediaPlayer::LoadedMedia:
		case QMediaPlayer::BufferedMedia:
			return MediaBackend::PlayingState;
		default:
			break;
		}
	}

	return MediaBackend::NotPlayingState;
}

qint64
MediaBackend::progress() const
{
	return d->position();
}

qint64
MediaBackend::duration() const
{
	return d->duration();
}

void
MediaBackend::play(MediaSource *source)
{
	if (source) {
		d->_metaDataInvalid = true;
		d->setMedia(*source);
	}
	d->play();
}

void
MediaBackend::pause()
{
	if (d->media().contentUri().scheme() == "file")
		d->pause();
	else
		d->stop();
}

bool
MediaBackend::isSeekable() const
{
	return d->isSeekable();
}

void
MediaBackend::push(MediaSource *source)
{
	play(source);
}

void
MediaBackend::seek(qint64 position)
{
	d->setPosition(position);
}

QString
MediaBackend::metaData(MetaData genericKey) const
{
	QAbstractMediaObject::MetaData key;
	switch (genericKey) {
	case MediaBackend::Title:
		key = QAbstractMediaObject::Title;
		break;
	case MediaBackend::Artist:
		key = QAbstractMediaObject::AlbumArtist;
		break;
	case MediaBackend::Album:
		key = QAbstractMediaObject::AlbumTitle;
		break;
	case MediaBackend::Date:
		key = QAbstractMediaObject::Year;
		break;
	default:
		return QString();
	}
	return d->metaData(key).toString();
}

MediaSource *
MediaBackend::createSource(const QString &uri)
{
	if (uri.startsWith('/'))
		return new MediaSource(QUrl("file://" + uri));
	else
		return new MediaSource(QUrl(uri));
}

void
MediaBackend::deleteSource(MediaSource *source)
{
	delete source;
}

QString
MediaBackend::sourceUrl(MediaSource *source)
{
	return source->contentUri().toString();
}

#endif // USE_QTMEDIA
