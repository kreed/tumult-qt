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

#include "player.h"
#include "streamnode.h"
#include "streamsmodel.h"
#include <qboxlayout.h>
#include <qheaderview.h>
#include <qpushbutton.h>
#include <qtableview.h>
#include <qtabwidget.h>

SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	QTabWidget *tabWidget = new QTabWidget;

	QWidget *streamsTab = new QWidget;
	QGridLayout *grid = new QGridLayout;
	grid->setColumnStretch(0, 1);
	grid->setRowStretch(4, 1);

	_streamsModel = new StreamsModel(this);
	_streamsView = new QTableView;
	_streamsView->setModel(_streamsModel);
	_streamsView->horizontalHeader()->resizeSection(0, 160);
	_streamsView->horizontalHeader()->setStretchLastSection(true);
	_streamsView->verticalHeader()->hide();
	_streamsView->setShowGrid(false);
	_streamsView->setAlternatingRowColors(true);
	_streamsView->setSelectionBehavior(QAbstractItemView::SelectRows);
	grid->addWidget(_streamsView, 0, 0, -1, 1);

	QPushButton *add = new QPushButton("Add");
	connect(add, SIGNAL(clicked()),
	             SLOT(addStream()));
	grid->addWidget(add, 0, 1);

	QPushButton *remove = new QPushButton("Remove");
	connect(remove, SIGNAL(clicked()),
	                SLOT(removeStream()));
	grid->addWidget(remove, 1, 1);

	QPushButton *up = new QPushButton("Move Up");
	connect(up, SIGNAL(clicked()),
	            SLOT(moveUp()));
	grid->addWidget(up, 2, 1);

	QPushButton *down = new QPushButton("Move Down");
	connect(down, SIGNAL(clicked()),
	              SLOT(moveDown()));
	grid->addWidget(down, 3, 1);

	streamsTab->setLayout(grid);
	tabWidget->addTab(streamsTab, "Streams");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(tabWidget);
	setLayout(layout);
}

QSize
SettingsDialog::sizeHint() const
{
	return QSize(600, 300);
}

void
SettingsDialog::addStream()
{
	_streamsModel->insertRow(_streamsModel->rowCount());
}

void
SettingsDialog::removeStream()
{
	foreach (const QModelIndex &index, _streamsView->selectionModel()->selectedRows())
		_streamsModel->removeRow(index.row());
}

void
SettingsDialog::moveUp()
{
	QModelIndexList row = _streamsView->selectionModel()->selectedRows();
	if (row.size() != 1)
		return;

	_streamsModel->move(row.first().row(), StreamsModel::Up);
}

void
SettingsDialog::moveDown()
{
	QModelIndexList row = _streamsView->selectionModel()->selectedRows();
	if (row.size() != 1)
		return;

	_streamsModel->move(row.first().row(), StreamsModel::Down);
}
