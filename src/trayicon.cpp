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

#include "trayicon.h"

#include "player.h"
#include "settingsdialog.h"
#include <qapplication.h>
#include <qmenu.h>

TrayIcon::TrayIcon()
	: QSystemTrayIcon(QIcon(":icon.svg"))
{
	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
	              SLOT(activate(QSystemTrayIcon::ActivationReason)));

	QMenu *menu = new QMenu;
	Player *player = Player::instance;

	menu->addAction(player->showStatusAction);

	menu->addSeparator();

	menu->addAction(player->searchAction);
	menu->addAction(player->clearQueueAction);

	menu->addSeparator();

	menu->addAction(player->playPauseAction);
	menu->addAction(player->prevStreamAction);
	menu->addAction(player->nextStreamAction);
	menu->addAction(player->prevInStreamAction);
	menu->addAction(player->nextInStreamAction);

	menu->addSeparator();

	menu->addAction("Settings...", this, SLOT(showSettingsDialog()));

	menu->addSeparator();

	menu->addAction("Quit", qApp, SLOT(quit()), QKeySequence("Ctrl+Q"))->setShortcutContext(Qt::ApplicationShortcut);

	setContextMenu(menu);
}

void
TrayIcon::activate(ActivationReason reason)
{
	if (reason == Trigger)
		Player::instance->showStatus();
}

void
TrayIcon::showSettingsDialog()
{
	SettingsDialog *settings = new SettingsDialog;
	connect(settings, SIGNAL(finished(int)),
	        Player::instance, SLOT(save()));
	settings->setAttribute(Qt::WA_DeleteOnClose);
	settings->show();
}
