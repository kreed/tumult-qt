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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <qlabel.h>
#include <qbasictimer.h>

template<class Key, class T> class QMultiMap;

class MessageWindow : QLabel {
	Q_OBJECT
public:
	MessageWindow();

	void showText(const QString &text);
	void showMetadata(const QMultiMap<QString, QString>&);

	using QLabel::hide;

protected:
	void timerEvent(QTimerEvent *ev);

private:
	void show(int timeout);

	QBasicTimer _timer;
	bool _metadataShown;
};

#endif
