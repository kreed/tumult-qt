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

#ifndef PLAYER_H
#define PLAYER_H

#include <qbasictimer.h>
#include <qdatetime.h>
#include <qpointer.h>

class MediaBackend;
class MediaSource;
class MessageWindow;
class QAction;
class SearchBox;
class StreamNode;

class Player : public QObject {
	Q_OBJECT
public:
	Player();

	void changeSource(MediaSource *source);

	StreamNode *streamAt(int i) const;
	int indexOf(StreamNode *) const;
	int streamCount() const;

	void setStreamName(const QString &name);
	void setStreamLocation(const QString &location);

	static Player *instance;

	QAction* const showStatusAction;
	QAction* const searchAction;
	QAction* const playPauseAction;
	QAction* const prevStreamAction;
	QAction* const nextStreamAction;
	QAction* const prevInStreamAction;
	QAction* const nextInStreamAction;
	QAction* const clearQueueAction;

signals:
	void streamChanged(StreamNode *oldStream, StreamNode *newStream);

public slots:
	void save() const;

	void showStatus();
	void nextStream();
	void prevStream();
	void prevInStream();
	void nextInStream(bool force = false);
	void clearQueue();
	void openSearchBox();
	void playPause();
	void repopulateStream();

private slots:
	void loadAnother();
	void search();
	void saveHit(const QString &url);
	void newSourceLoaded();
	void newStreamName(StreamNode *);
	void streamDestroyed(QObject *);
	void updatePlaying(bool playing);

protected:
	void timerEvent(QTimerEvent *);

private:
	void setStream(StreamNode *stream, bool play = true);
	bool fixNoStream();
	bool fixEmptyOrStopped();

	MediaBackend *_backend;
	MessageWindow *_message;

	QPointer<SearchBox> _searchBox;
	QString _lastSearch;

	StreamNode *_firstStream;
	StreamNode *_currentStream;

	bool _showNextMetaData;
	qint64 _toSeek;

	QBasicTimer _saveTimer;
	mutable QDateTime _playStartTime;

	friend class StreamsModel;
};

#endif
