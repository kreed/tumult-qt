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

#include "keys.h"
#include "player.h"
#include "settings.h"
#include "tumult.h"
#include <qdbusconnection.h>

int
main(int argc, char **argv)
{
	qsrand(time(NULL));

	Tumult app(argc, argv);
	app.setQuitOnLastWindowClosed(false);
	app.setApplicationName("Tumult");

	player = new Player;
	keys = new Keys;
	Settings::parse();
	player->init();

	QDBusConnection bus = QDBusConnection::sessionBus();
	bus.registerService("org.tumult");
	bus.registerObject("/", player, QDBusConnection::ExportAllSlots);

	return app.exec();
}

