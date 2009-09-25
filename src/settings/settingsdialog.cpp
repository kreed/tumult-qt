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

#include "settingsdialog.h"

#include "streamsmodel.h"
#include <qboxlayout.h>
#include <qheaderview.h>
#include <qtableview.h>
#include <qtabwidget.h>

SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	QTabWidget *tabWidget = new QTabWidget;

	QTableView *streamsView = new QTableView;
	streamsView->setModel(new StreamsModel(this));
	streamsView->horizontalHeader()->setStretchLastSection(true);
	streamsView->verticalHeader()->hide();
	streamsView->setShowGrid(false);
	streamsView->setAlternatingRowColors(true);
	tabWidget->addTab(streamsView, "Streams");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(tabWidget);
	setLayout(layout);
}

QSize
SettingsDialog::sizeHint() const
{
	return QSize(500, 300);
}
