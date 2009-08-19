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

#include <phonon/mediaobject.h>
#include <qlinkedlist.h>
#include <qpointer.h>

class MessageWindow;
class SearchBox;
class Stream;
class QAction;
typedef QLinkedList<Stream*> StreamList;

class Player : public Phonon::MediaObject {
	Q_OBJECT
public:
	Player();

	Stream *currentStream() const { return *_currentStream; }

	void setCurrentSource(const Phonon::MediaSource&);
	void changeSource(const Phonon::MediaSource&);

	static Player *instance;

	QAction* const showStatusAction;
	QAction* const searchAction;
	QAction* const playPauseAction;
	QAction* const prevStreamAction;
	QAction* const nextStreamAction;
	QAction* const prevInStreamAction;
	QAction* const nextInStreamAction;
	QAction* const clearQueueAction;

public slots:
	void showStatus(bool metadata = true);
	void play();
	void nextStream();
	void prevStream();
	void prevInStream();
	void nextInStream();
	void clearQueue();
	void openSearchBox();
	void smartStop();
	void playPause();
	void repopulateStream();

private slots:
	void loadAnother();
	void search();
	void newState(Phonon::State, Phonon::State);
	void newSource(const Phonon::MediaSource&);
	void setMetaData();

private:
	void saveHit();
	void setStream(const StreamList::const_iterator &stream);
	bool checkEmptyStream();

	StreamList _streams;
	StreamList::const_iterator _currentStream;
	MessageWindow *_message;
	QPointer<SearchBox> _searchBox;
	QString _lastSearch;
	bool _expectingSourceChange;
	QString _savedUrl;
	bool _metaDataInvalid;
	bool _showNextMetaData;
	qint64 _toSeek;
};

#endif
