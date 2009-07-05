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

#include "tumult.h"

#include "keys.h"
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>

bool
Tumult::x11EventFilter(XEvent *ev)
{
	if (ev->type == KeyPress)
		return Keys::instance->event(((XKeyEvent*)ev)->keycode, ((XKeyEvent*)ev)->state);

	return false;
}

bool
Tumult::isIdle(int secs)
{
	static XSyncCounter idle = 0;
	if (idle == -1)
		return false;

	Display *dpy = QX11Info::display();

	if (idle == None) {
		int xsync_major = SYNC_MAJOR_VERSION;
	    int xsync_minor = SYNC_MINOR_VERSION;
		int i;
		if (XSyncQueryExtension(dpy, &i, &i) && XSyncInitialize(dpy, &xsync_major, &xsync_minor)) {
			XSyncSystemCounter *counters = XSyncListSystemCounters(dpy, &i);
			while (i--)
				if (!strcmp(counters[i].name, "IDLETIME"))
					idle = counters[i].counter;
			XSyncFreeSystemCounterList(counters);
			if (idle)
				goto test;
		}

		idle = -1;
		return false;
	}

test:
	XSyncValue threshold;
	XSyncValue idleTime;
	XSyncIntToValue(&threshold, secs * 1000);
	XSyncQueryCounter(dpy, idle, &idleTime);
	return XSyncValueLessThan(threshold, idleTime);
}
