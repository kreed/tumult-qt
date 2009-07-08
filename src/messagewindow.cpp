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
#include <qdatetime.h>
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

	show();
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
MessageWindow::showMetaData()
{
	int timeout = 3500;

	// more detailed information on duplicate request
	if (_metadataShown && isVisible()) {
		// if Phonon gains support for it, add cover art here
		setAlignment(Qt::AlignLeft);
		setText(QString(
			"<b>Title: </b>%1<br>"
			"<b>Artist: </b>%2<br>"
			"<b>Album: </b>%3<br>"
			"<b>Date: </b>%4<br>"
			"<b>Progress: </b>%5 / %6<br>"
			"<b>Stream: </b>%7")
			.arg(_title
			   , _artist
			   , _album
			   , _date
			   , QTime().addMSecs(_currentTime).toString("m:ss")
			   , _totalTime
			   , _stream));
		timeout = 5000;
	} else {
		setAlignment(Qt::AlignCenter);
		if (!_title.isEmpty() || !_artist.isEmpty())
			setText(QString("<b>%1</b><br>%2<br>%3")
			           .arg(_title, _artist, _stream));
		else
			setText(_url + "<br>" + _stream);
	}

	_metadataShown = true;
	show(timeout);
}

void
MessageWindow::setMetaData(const QMultiMap<QString, QString> &data)
{
	// FIXME: should use join for metadata
	_title = data.value("TITLE");
	_artist = data.value("ARTIST");
	_album = data.value("ALBUM");
	_date = data.value("DATE");

	if (_artist.isEmpty() && _title.contains(" - ")) {
		QStringList at = _title.split(" - ");
		_title = at[1];
		_artist = at[0];
	}
}

void
MessageWindow::setProgress(qint64 current, qint64 total)
{
	_currentTime = current;
	if (total)
		_totalTime = QTime().addMSecs(total).toString("m:ss");
}

void
MessageWindow::setStream(const QString &stream)
{
	_stream = stream;
}

void
MessageWindow::setUrl(const QString &url)
{
	_url = url;
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
