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

#ifdef USE_PHONON

#include "mediabackend.h"
#include "phonon_p.h"

#include <phonon/audiooutput.h>
#include <qstringlist.h>
#include <qurl.h>

class MediaSource : public Phonon::MediaSource {
public:
	MediaSource(const QString &uri)
		: Phonon::MediaSource(uri)
	{
	}
	MediaSource(const QUrl &uri)
		: Phonon::MediaSource(uri)
	{
	}
};

MediaBackendPrivate::MediaBackendPrivate(MediaBackend *parent)
	: Phonon::MediaObject(parent)
	, q(parent)
	, _expectingSourceChange(false)
	, _metaDataInvalid(false)
{
	connect(this, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
	              SLOT(newState(Phonon::State, Phonon::State)));
	connect(this, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)),
	              SLOT(newSource(const Phonon::MediaSource&)));
	connect(this, SIGNAL(metaDataChanged()),
	              SLOT(newMetaData()));
}

void
MediaBackendPrivate::newState(Phonon::State news, Phonon::State olds)
{
	Q_UNUSED(olds);
	if (news == Phonon::PlayingState)
		emit q->sourceLoaded();
	emit q->playingChanged(news == Phonon::PlayingState);
}

void
MediaBackendPrivate::newSource(const Phonon::MediaSource &src)
{
	if (_expectingSourceChange)
		_expectingSourceChange = false;
	else
		emit q->sourceFinished(_savedUrl);

	_savedUrl = src.url().toString();
	_metaDataInvalid = true;
	emit q->sourceChanged(_savedUrl);
}

void
MediaBackendPrivate::newMetaData()
{
	_metaDataInvalid = false;
	emit q->metaDataChanged(q);
}

MediaBackend::MediaBackend(QObject *parent)
	: QObject(parent)
	, d(new MediaBackendPrivate(this))
{
	Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	audioOutput->setName("Tumult");
	Phonon::createPath(d, audioOutput);
}

bool
MediaBackend::isMetaDataInvalid() const
{
	return d->_metaDataInvalid;
}

bool
MediaBackend::isPlaying() const
{
	return d->state() == Phonon::PlayingState;
}

bool
MediaBackend::isSourceNull() const
{
	return d->currentSource().type() == Phonon::MediaSource::Empty;
}

QString
MediaBackend::errorString() const
{
	return d->errorString();
}

MediaBackend::State
MediaBackend::state() const
{
	switch (d->state()) {
	case Phonon::BufferingState:
		return MediaBackend::BufferingState;
	case Phonon::PlayingState:
		return MediaBackend::PlayingState;
	case Phonon::LoadingState:
		if (!isSourceNull())
			return MediaBackend::LoadingState;
		// else we haven't loaded anything yet since the stream has just been created; don't return loading
	default:
		return MediaBackend::NotPlayingState;
	}
}

qint64
MediaBackend::progress() const
{
	return d->currentTime();
}

qint64
MediaBackend::duration() const
{
	return d->totalTime();
}

void
MediaBackend::play(MediaSource *source)
{
	if (source) {
		d->_expectingSourceChange = true;
		d->setCurrentSource(*source);
	}
	d->clearQueue();
	d->play();
}

void
MediaBackend::pause()
{
	if (d->currentSource().type() == Phonon::MediaSource::LocalFile)
		d->pause();
	else
		d->stop();
}

bool
MediaBackend::isSeekable() const
{
	return d->isSeekable() && d->currentSource().type() == Phonon::MediaSource::LocalFile;
}

void
MediaBackend::push(MediaSource *source)
{
	d->enqueue(*source);
}

void
MediaBackend::seek(qint64 position)
{
	d->seek(position);
}

QString
MediaBackend::metaData(MetaData genericKey) const
{
	Phonon::MetaData key;
	switch (genericKey) {
	case MediaBackend::Title:
		key = Phonon::TitleMetaData;
		break;
	case MediaBackend::Artist:
		key = Phonon::ArtistMetaData;
		break;
	case MediaBackend::Album:
		key = Phonon::AlbumMetaData;
		break;
	case MediaBackend::Date:
		key = Phonon::DateMetaData;
		break;
	default:
		return QString();
	}
	return d->metaData(key).join(", ");
}

MediaSource *
MediaBackend::createSource(const QString &uri)
{
	if (uri.startsWith('/'))
		return new MediaSource(uri);
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
	return source->url().toString();
}

#endif // USE_PHONON
