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

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qmap.h>

MessageWindow::MessageWindow()
{
	_metadataShown = false;

	setWindowFlags(Qt::X11BypassWindowManagerHint);
	setMaximumWidth(300);
	setWordWrap(true);
	setMargin(5);

	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::black);
	p.setColor(QPalette::WindowText, Qt::white);
	setPalette(p);
}

void
MessageWindow::show(int timeout)
{
	_timer.start(timeout, this);

	adjustSize();
	move(QApplication::desktop()->width() - width() - 5, 5);

	QLabel::show();
	raise();
}

void
MessageWindow::showText(const QString &text)
{
	setText(text);
	_metadataShown = false;
	show(3500);
}

void
MessageWindow::showMetadata(const QMultiMap<QString, QString> &data)
{
	// FIXME: should use join for metadata
	int timeout = 3500;

	QString title = data.value("TITLE");
	QString artist = data.value("ARTIST");

	if (artist.isEmpty() && title.contains(" - ")) {
		QStringList at = title.split(" - ");
		title = at[1];
		artist = at[0];
	}

	// more detailed information on duplicate request
	if (_metadataShown && isVisible()) {
		// if Phonon gains support for it, add cover art here
		// ditto for year
		setAlignment(Qt::AlignLeft);
		setText(QString(
			"<b>Title: </b>%1<br>"
			"<b>Artist: </b>%2<br>"
			"<b>Album: </b>%3<br>"
			"<b>Date: </b>%4<br>"
			"<b>Genre: </b>%5<br>"
			"<b>Progress: </b>%6<br>"
			"<b>Stream: </b>%7")
			.arg(
			     title
			   , artist
			   , data.value("ALBUM")
			   , data.value("DATE")
			   , data.value("GENRE")
			   , data.value("PROGRESS")
			   , data.value("STREAM")));
		timeout = 5000;
	} else {
		setAlignment(Qt::AlignCenter);
		if (!title.isEmpty() || !artist.isEmpty())
			setText(QString("<b>%1</b><br>%2<br>%3")
			           .arg(title, artist, data.value("STREAM")));
		else
			setText(data.value("URL") + "<br>" + data.value("STREAM"));
	}

	_metadataShown = true;
	show(timeout);
}

void
MessageWindow::timerEvent(QTimerEvent *ev)
{
	if (ev->timerId() == _timer.timerId()) {
		_timer.stop();
		hide();
	}
	QLabel::timerEvent(ev);
}
