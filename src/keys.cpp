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
#include <qsettings.h>
#include <qx11info_x11.h>
#include <X11/Xlib.h>

Keys *Keys::instance;

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
	if (keyCodes[action] || v.isEmpty())
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

		if (mods.contains("<alt>") || mods.contains("<mod1>"))
			modMasks[action] |= Mod1Mask;
		if (mods.contains("<shift>"))
			modMasks[action] |= ShiftMask;
		if (mods.contains("<ctrl>") || mods.contains("<control>"))
			modMasks[action] |= ControlMask;
		if (mods.contains("<win>") || mods.contains("<super>") || mods.contains("<mod4>"))
			modMasks[action] |= Mod4Mask;
		if (mods.contains("<mod5>"))
			modMasks[action] |= Mod5Mask;
		if (mods.contains("<any>"))
			modMasks[action] = 0;
	}

	errorCount = 0;
	XErrorHandler savedErrorHandler = XSetErrorHandler(catch_error);
	Window root = QX11Info::appRootWindow();

	if (modMasks[action]) {
		XGrabKey(dpy, keyCodes[action], modMasks[action], root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | LockMask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod2Mask, root,
				True, GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keyCodes[action], modMasks[action] | Mod2Mask | LockMask, root,
				True, GrabModeAsync, GrabModeAsync);
	} else
		XGrabKey(dpy, keyCodes[action], AnyModifier, root,
				True, GrabModeAsync, GrabModeAsync);

	XSync(dpy, 0);

	XSetErrorHandler(savedErrorHandler);
	if (errorCount)
		qWarning() << "Could not grab key" << v;
}

Keys::Keys()
{
	memset(keyCodes, 0, sizeof(keyCodes));
	memset(modMasks, 0, sizeof(modMasks));

	QSettings settings;
	settings.beginGroup("keys");
	parse(Next, settings.value("next_stream").toByteArray());
	parse(Prev, settings.value("prev_stream").toByteArray());
	parse(ShowStatus, settings.value("status").toByteArray());
	parse(PlayPause, settings.value("play_pause").toByteArray());
	parse(Search, settings.value("search").toByteArray());
	parse(NextInStream, settings.value("next_in_stream").toByteArray());
	parse(NextInQueue, settings.value("next_in_queue").toByteArray());

	instance = this;
}

static void
action(Keys::Action action)
{
	switch (action) {
	case Keys::ShowStatus:
		Player::instance->showStatus(true);
		break;
	case Keys::NextInStream:
		Player::instance->nextInStream();
		break;
	case Keys::PlayPause:
		Player::instance->playPause();
		break;
	case Keys::NextInQueue:
		Player::instance->nextInQueue();
		break;
	case Keys::Search:
		Player::instance->openSearchBox();
		break;
	case Keys::Prev:
		Player::instance->prevStream();
		break;
	case Keys::Next:
		Player::instance->nextStream();
		break;
	default:
		break;
	}
}

bool
Keys::event(unsigned keyCode, unsigned modMask)
{
	modMask &= ~(Mod2Mask | LockMask);
	for (int i = ActionCount; --i != -1;)
		if (keyCodes[i] == keyCode && (modMasks[i] == AnyModifier || modMask == modMasks[i])) {
			action((Action)i);
			return true;
		}

	return false;
}

