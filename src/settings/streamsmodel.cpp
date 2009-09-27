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

#include "streamsmodel.h"

#include "player.h"
#include "streamnode.h"
#include <qfont.h>

StreamsModel::StreamsModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	connect(Player::instance, SIGNAL(streamChanged(StreamNode *, StreamNode *)),
	                          SLOT(newStream(StreamNode *, StreamNode *)));
}

static StreamNode *
streamAt(int i)
{
	StreamNode *first = Player::instance->firstStream();
	StreamNode *stream = first;

	for (int j = 0; j != i; ++j) {
		stream = stream->nextStream();
		if (stream == first)
			return NULL;
	}

	return stream;
}

static int
indexOf(StreamNode *needle)
{
	StreamNode *first = Player::instance->firstStream();
	StreamNode *stream = first;

	for (int i = 0; ; ++i) {
		if (stream == needle)
			return i;
		stream = stream->nextStream();
		if (stream == first)
			return -1;
	}
}

// this only inserts streams at the end
bool
StreamsModel::insertRows(int row, int count, const QModelIndex &parent)
{
	if (count < 1 || parent.isValid())
		return false;

	row = rowCount();
	int lastRow = row + count - 1;
	beginInsertRows(parent, row, lastRow);

	StreamNode *stream = streamAt(0)->prevStream();

	while (--count >= 0)
		stream = new StreamNode("New Stream", QString(), stream);

	endInsertRows();
	return true;
}

bool
StreamsModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (row < 0 || count <= 0 || count >= rowCount() || parent.isValid())
		return false;

	int lastRow = row + count - 1;
	StreamNode *stream = streamAt(row);
	if (!stream)
		return false;

	beginRemoveRows(parent, row, lastRow);

	while (--count >= 0) {
		StreamNode *next = stream->nextStream();
		delete stream;
		stream = next;
	}

	endRemoveRows();
	return true;
}

int
StreamsModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	StreamNode *first = Player::instance->firstStream();
	StreamNode *stream = first;
	int count = 1;

	while (stream->nextStream() != first) {
		stream = stream->nextStream();
		++count;
	}

	return count;
}

int
StreamsModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 2;
}

Qt::ItemFlags
StreamsModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant
StreamsModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= rowCount())
		return QVariant();

	StreamNode *stream = streamAt(index.row());
	if (!stream)
		return QVariant();

	switch (index.column()) {
	case 0:
		switch (role) {
		case Qt::EditRole:
		case Qt::DisplayRole:
			return stream->name();
		}
		break;
	case 1:
		switch (role) {
		case Qt::EditRole:
		case Qt::DisplayRole:
			return stream->location();
		}
		break;
	}

	if (role == Qt::FontRole) {
		QFont font;
		font.setBold(Player::instance->_currentStream == stream);
		return font;
	}

	return QVariant();
}

bool
StreamsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.row() < 0 || role != Qt::EditRole)
		return false;

	StreamNode *stream = streamAt(index.row());
	if (!stream)
		return false;

	switch (index.column()) {
	case 0:
		stream->setName(value.toString());
		break;
	case 1:
		stream->setLocation(value.toString());
		break;
	default:
		return false;
	}

	return true;
}

QVariant
StreamsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Vertical || role != Qt::DisplayRole)
		return QVariant();

	switch (section) {
	case 0:
		return "Name";
	case 1:
		return "Source";
	}

	return QVariant();
}

bool
StreamsModel::move(int row, Direction direction)
{
	int rows = rowCount();
	if (row < 0 || row >= rows)
		return false;
	if (direction == Up && row == 0)
		return false;
	if (direction == Down && row == rows - 1)
		return false;

	int destination = row + direction;
	int offset = direction == Down ? 1 : 0;
	beginMoveRows(QModelIndex(), row, row, QModelIndex(), destination + offset);

	StreamNode *stream = streamAt(row);
	StreamNode *destStream = streamAt(destination);
	StreamNode::swap(stream, destStream);

	if (row == 0)
		Player::instance->_firstStream = destStream;
	if (destination == 0)
		Player::instance->_firstStream = stream;

	endMoveRows();
	return true;
}

void
StreamsModel::newStream(StreamNode *oldStream, StreamNode *newStream)
{
	int rOld = indexOf(oldStream);
	int rNew = indexOf(newStream);

	int lower = qMin(rOld, rNew);
	int upper = qMax(rOld, rNew);
	emit dataChanged(createIndex(lower, 0), createIndex(upper, 1));
}
