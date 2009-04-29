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

#include "searchbox.h"

#include <qcursor.h>
#include <qevent.h>

SearchBox::SearchBox()
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(Qt::Dialog);
	setFocus();
	adjustSize();
	connect(this, SIGNAL(editingFinished()),
	              SLOT(close()));
}

void
SearchBox::search(const QString &lastText)
{
	setText(lastText);
	selectAll();

	QPoint pos = QCursor::pos();
	pos.rx() -= width() / 2;
	pos.ry() -= height() / 2;
	move(pos);

	show();
	activateWindow();
}

void
SearchBox::keyReleaseEvent(QKeyEvent *ev)
{
	if (ev->key() == Qt::Key_Escape) {
		close();
		ev->accept();
	}
	QLineEdit::keyReleaseEvent(ev);
}
