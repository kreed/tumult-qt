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

#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QLineEdit>

class StreamElement;

class SearchBox : public QLineEdit {
	Q_OBJECT
public:
	SearchBox();

	void search(StreamElement *e);

protected slots:
	void searchCallback();

protected:
	void keyReleaseEvent(QKeyEvent*);

private:
	StreamElement *_stream;
	bool _forceSearch;
};

#endif
