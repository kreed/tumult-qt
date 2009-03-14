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

#include "messagewindow.h"

#include <QApplication>
#include <QDesktopWidget>

MessageWindow::MessageWindow()
	: _timerId(0)
{
	setWindowFlags(Qt::X11BypassWindowManagerHint);
	setAlignment(Qt::AlignCenter);
	setMargin(5);

	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	p.setColor(QPalette::WindowText, Qt::white);
	setPalette(p);
}

void
MessageWindow::showMessage(const QString &text)
{
	if (_timerId)
		killTimer(_timerId);

	setText(text);

	adjustSize();
	move(QApplication::desktop()->width() - width() - 5, 5);

	show();
	raise();

	_timerId = startTimer(3500);
}

void
MessageWindow::timerEvent(QTimerEvent *ev)
{
	if (ev->timerId() == _timerId) {
		killTimer(_timerId);
		_timerId = 0;
		hide();
	}
	QLabel::timerEvent(ev);
}
