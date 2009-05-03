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
#include <X11/Xlib.h>

bool
Tumult::x11EventFilter(XEvent *ev)
{
	if (ev->type == KeyPress)
		return Keys::instance->event(((XKeyEvent*)ev)->keycode, ((XKeyEvent*)ev)->state);

	return false;
}
