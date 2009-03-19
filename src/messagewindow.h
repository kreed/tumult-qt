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

class MessageWindow : QLabel {
	Q_OBJECT
public:
	enum Mode {
		Info,
		MetaDataBrief,
		MetaDataVerbose
	};

	MessageWindow();

	void showText(const QString &text, Mode mode = Info);

	bool verboseReady() const { return _verbose && isVisible(); }
	void clearVerboseFlag() { _verbose = false; }

protected:
	void timerEvent(QTimerEvent *ev);

private:
	int _timerId;
	bool _verbose;
};

#endif
