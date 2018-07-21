/*
 * Copyright (C) 2012 Stellarium Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */

#include "PropertyBasedTableModel_Point.hpp"
#include <QDebug>

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark instance Methods
#endif
/* ********************************************************************* */
PropertyBasedTableModel_Point::PropertyBasedTableModel_Point(QObject *parent)
	: QAbstractTableModel(parent)
	, content(Q_NULLPTR)
	, modelObject(Q_NULLPTR)
{
}


PropertyBasedTableModel_Point::~PropertyBasedTableModel_Point()
{
	delete modelObject;
	modelObject = Q_NULLPTR;
}

void PropertyBasedTableModel_Point::init(QList<QObject *>* content, QObject *model, QMap<int,QString> mappings)
{
	beginResetModel();
	this->content = content;
	this->modelObject = model;
	this->mappings = mappings;
	endResetModel();
}

/* ********************************************************************* */
#if 0
#pragma mark -
#pragma mark Model Methods
#endif
/* ********************************************************************* */

int PropertyBasedTableModel_Point::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return content->size();
}

int PropertyBasedTableModel_Point::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return mappings.size();
}

QVariant PropertyBasedTableModel_Point::data(const QModelIndex &index, int role) const
{
	QVariant data;
	if ((role == Qt::DisplayRole || role == Qt::EditRole)
		 && index.isValid()
		 && index.row() < content->size()
		 && index.row() >= 0
		 && index.column() < mappings.size()
		 && index.column() >= 0){
			QObject *object = content->at(index.row());
			data = object->property(mappings[index.column()].toStdString().c_str());
  }
	return data;
}

bool PropertyBasedTableModel_Point::insertRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	for (int row=0; row < rows; row++) {
		QObject* newInstance = modelObject->metaObject()->newInstance(Q_ARG(QObject, *modelObject));
		Q_ASSERT(newInstance != 0);
		content->insert(position, newInstance);
	}

	endInsertRows();
	return true;
}

bool PropertyBasedTableModel_Point::removeRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginRemoveRows(QModelIndex(), position, position + rows - 1);

	for (int row=0; row < rows; ++row) {
		content->removeAt(position);
	}

	endRemoveRows();
	return true;
}

bool PropertyBasedTableModel_Point::setData(const QModelIndex &index, const QVariant &value, int role)
{
	bool changeMade = false;
	if (index.isValid() && role == Qt::EditRole && index.column() < mappings.size()) {
		QObject* object = content->at(index.row());
		object->setProperty(mappings[index.column()].toStdString().c_str(), value);
		emit(QAbstractItemModel::dataChanged(index, index));

		changeMade = true;
	}

	return changeMade;
}

Qt::ItemFlags PropertyBasedTableModel_Point::flags(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

void PropertyBasedTableModel_Point::moveRowUp(int position)
{
	int count = content->count();
	if (count < 2 || position < 1 || position >= count)
		return;

	beginMoveRows(QModelIndex(), position, position, QModelIndex(), position-1);

	content->move(position, position - 1);

	endMoveRows();
}

void PropertyBasedTableModel_Point::moveRowDown(int position)
{
	int count = content->count();
	if (count < 2 || position < 0 || position > (count - 2))
		return;

	beginMoveRows(QModelIndex(), position, position, QModelIndex(), position+2);

	content->move(position, position + 1);

	endMoveRows();
}

