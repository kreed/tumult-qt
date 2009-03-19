/*
 * Copyright © 2008-2009 Christopher Eby <kreed@kreed.org>
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

#include "keys.h"

#include "player.h"
#include <qx11info_x11.h>
#include "settings.h"
#include <X11/Xlib.h>

Keys *keys;
static int errorCount = 0;

static int
catch_error(Display*, XErrorEvent*)
{
	++errorCount;
	return 0;
}

void
Keys::parse(Action action, const QByteArray &v)
{
	if (keyCodes[action])
		return;

	Display *dpy = QX11Info::display();

	int i = v.lastIndexOf('>') + 1;

	QByteArray key = v.mid(i);

	KeySym sym = XStringToKeysym(key.constData());
	if (sym == NoSymbol) {
		qWarning() << "Error parsing key" << v;
		return;
	}

	keyCodes[action] = XKeysymToKeycode(dpy, sym);

	if (i) {
		QByteArray mods = v.left(i).toLower();

		if (mods.contains("<alt>"))
			modMasks[action] |= Mod1Mask;
		if (mods.contains("<shift>"))
			modMasks[action] |= ShiftMask;
		if (mods.contains("<ctrl>") || mods.contains("<control>"))
			modMasks[action] |= ControlMask;
		if (mods.contains("<win>") || mods.contains("<super>"))
			modMasks[action] |= Mod5Mask;
		if (mods.contains("<any>"))
			modMasks[action] = AnyModifier;
	}

	errorCount = 0;
	XErrorHandler savedErrorHandler = XSetErrorHandler(catch_error);
	Window root = QX11Info::appRootWindow();

	if (modMasks[action]) {
		XGrabKey(dpy, keyCodes[action], modMasks[action], root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod5Mask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | LockMask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod2Mask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod2Mask | Mod5Mask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod2Mask | LockMask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod5Mask | LockMask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod5Mask | LockMask | Mod2Mask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action], root,
				True, GrabModeAsync, GrabModeAsync);
	} else
		XGrabKey(dpy, keyCodes[action], AnyModifier, root,
				True, GrabModeAsync, GrabModeAsync);

	XSync(dpy, 0);

	XSetErrorHandler(savedErrorHandler);
	if (errorCount)
		qWarning() << "Could not grab key" << v;
}

#define KEY(function, action) \
	void \
	function(const QByteArray &v) \
	{ \
		keys->parse(Keys::action, v); \
	}

KEY(next, Next)
KEY(prev, Prev)
KEY(status, ShowStatus)
KEY(play_pause, PlayPause)
KEY(search, Search)
KEY(next_in_stream, NextInStream)

Keys::Keys()
{
	memset(keyCodes, 0, sizeof(keyCodes));
	memset(modMasks, 0, sizeof(modMasks));

	Settings::add("next_stream", next);
	Settings::add("prev_stream", prev);
	Settings::add("status", status);
	Settings::add("play_pause", play_pause);
	Settings::add("search", search);
	Settings::add("next_in_stream", next_in_stream);
}

static void
action(Keys::Action action)
{
	switch (action) {
	case Keys::ShowStatus:
		player->showMetaData();
		break;
	case Keys::NextInStream:
		player->nextInStream();
		break;
	case Keys::PlayPause:
		player->playPause();
		break;
	case Keys::Search:
		player->openSearchBox();
		break;
	case Keys::Prev:
		player->prevStream();
		break;
	case Keys::Next:
		player->nextStream();
		break;
	default:
		break;
	}
}

bool
Keys::event(unsigned keyCode, unsigned modMask)
{
	modMask &= ~(Mod2Mask | Mod5Mask | LockMask);
	for (int i = 0; i != ActionCount; ++i)
		if (keyCodes[i] == keyCode && (modMasks[i] == AnyModifier || modMask == modMasks[i])) {
			action((Action)i);
			return true;
		}

	return false;
}

