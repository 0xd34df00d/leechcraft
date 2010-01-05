/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <algorithm>
#include <typeinfo>
#include <stdexcept>
#include <QtDebug>
#include "mergemodel.h"
#ifdef QT_DEBUG
#include "modeltest.h"
#endif

using namespace LeechCraft::Util;

MergeModel::MergeModel (const QStringList& headers, QObject *parent)
: QAbstractProxyModel (parent)
, DefaultAcceptsRowImpl_ (false)
, Headers_ (headers)
{
#ifdef QT_DEBUG
	new ModelTest (this, this);
#endif
}

MergeModel::~MergeModel ()
{
}

int MergeModel::columnCount (const QModelIndex& index) const
{
	if (index.isValid ())
	{
		QModelIndex mapped = mapToSource (index);
		return mapped.model ()->columnCount (mapped);
	}
	else
		return Headers_.size ();
}

QVariant MergeModel::headerData (int column, Qt::Orientation orient, int role) const
{
	if (orient != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant ();

	return Headers_.at (column);
}

QVariant MergeModel::data (const QModelIndex& index, int role) const
{
	if (index.isValid ())
	{
		QModelIndex mapped = mapToSource (index);
		return mapped.data (role);
	}
	else
		return QVariant ();
}

Qt::ItemFlags MergeModel::flags (const QModelIndex& index) const
{
	QModelIndex mapped = mapToSource (index);
	return mapped.flags ();
}

QModelIndex MergeModel::index (int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid ())
	{
		QModelIndex mapped = mapToSource (parent);
		return mapped.model ()->index (row, column, mapped);
	}
	else
	{
		if (!hasIndex (row, column))
			return QModelIndex ();

		return createIndex (row, column);
	}
}

QModelIndex MergeModel::parent (const QModelIndex&) const
{
	// here goes blocker for hierarchical #1
	return QModelIndex ();
}

int MergeModel::rowCount (const QModelIndex& parent) const
{
	if (!parent.isValid ())
	{
		int result = 0;
		for (models_t::const_iterator i = Models_.begin (),
				end = Models_.end ();
				i != end; ++i)
			result += RowCount (*i);
		return result;
	}
	else
	{
		QModelIndex mapped = mapToSource (parent);
		return mapped.model ()->rowCount (mapped);
	}
}

QModelIndex MergeModel::mapFromSource (const QModelIndex& sourceIndex) const
{
	if (!sourceIndex.isValid ())
		return QModelIndex ();

	const QAbstractItemModel *model = sourceIndex.model ();
	const_iterator moditer = FindModel (model);

	int startingRow = GetStartingRow (moditer);

	int sourceRow = sourceIndex.row ();
	int sourceColumn = sourceIndex.column ();
	void *sourcePtr = sourceIndex.internalPointer ();
	quint32 sourceId = sourceIndex.internalId ();

	if (sourcePtr)
		return createIndex (sourceRow + startingRow, sourceColumn, sourcePtr);
	else
		return createIndex (sourceRow + startingRow, sourceColumn, sourceId);
}

QModelIndex MergeModel::mapToSource (const QModelIndex& proxyIndex) const
{
	if (!proxyIndex.isValid ())
		return QModelIndex ();

	int proxyRow = proxyIndex.row ();
	int proxyColumn = proxyIndex.column ();
	const_iterator modIter;
	try
	{
		modIter = GetModelForRow (proxyRow);
	}
	catch (const std::runtime_error& e)
	{
		QStringList models;
		Q_FOREACH (QAbstractItemModel *model, Models_)
			models << model->objectName ();
		qWarning () << Q_FUNC_INFO
			<< "\n"
			<< objectName ()
			<< proxyIndex
			<< "\n"
			<< e.what ()
			<< "\n"
			<< models;
		throw;
	}
	//
	// here goes blocker for hierarchical #2
	int startingRow = GetStartingRow (modIter);

	return (*modIter)->index (proxyRow - startingRow, proxyColumn, QModelIndex ());
}

void MergeModel::setSourceModel (QAbstractItemModel*)
{
	throw std::runtime_error ("You should not set source model via setSourceModel()");
}

void MergeModel::SetHeaders (const QStringList& headers)
{
	Headers_ = headers;
}

void MergeModel::AddModel (QAbstractItemModel *model)
{
	if (!model)
		return;

	int rows = RowCount (model);
	bool wouldInsert = false;
	if (rows > 0)
		wouldInsert = true;

	if (wouldInsert)
		beginInsertRows (QModelIndex (), rowCount (), rowCount () + rows - 1);
	Models_.push_back (model);
	connect (model,
			SIGNAL (columnsAboutToBeInserted (const QModelIndex&, int, int)),
			this,
			SLOT (handleColumnsAboutToBeInserted (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (columnsAboutToBeRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (handleColumnsAboutToBeRemoved (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (columnsInserted (const QModelIndex&, int, int)),
			this,
			SLOT (handleColumnsInserted (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (columnsRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (handleColumnsRemoved (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (handleDataChanged (const QModelIndex&, const QModelIndex&)));
	connect (model,
			SIGNAL (layoutAboutToBeChanged ()),
			this,
			SIGNAL (layoutAboutToBeChanged ()));
	connect (model,
			SIGNAL (layoutChanged ()),
			this,
			SIGNAL (layoutChanged ()));
	connect (model,
			SIGNAL (modelAboutToBeReset ()),
			this,
			SIGNAL (modelReset ()));
	connect (model,
			SIGNAL (rowsAboutToBeInserted (const QModelIndex&, int, int)),
			this,
			SLOT (handleRowsAboutToBeInserted (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (rowsAboutToBeRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (handleRowsAboutToBeRemoved (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (rowsInserted (const QModelIndex&, int, int)),
			this,
			SLOT (handleRowsInserted (const QModelIndex&, int, int)));
	connect (model,
			SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
			this,
			SLOT (handleRowsRemoved (const QModelIndex&, int, int)));
	if (wouldInsert)
		endInsertRows ();
}

MergeModel::const_iterator MergeModel::FindModel (const QAbstractItemModel *model) const
{
	return std::find (Models_.begin (), Models_.end (), model);
}

MergeModel::iterator MergeModel::FindModel (const QAbstractItemModel *model)
{
	return std::find (Models_.begin (), Models_.end (), model);
}

void MergeModel::RemoveModel (QAbstractItemModel *model)
{
	models_t::iterator i = FindModel (model);

	if (i == Models_.end ())
	{
		qWarning () << Q_FUNC_INFO << "not found model" << model;
		return;
	}

	int rows = RowCount (model);
	bool wouldRemove = false;
	if (rows > 0)
		wouldRemove = true;

	if (wouldRemove)
	{
		int startingRow = GetStartingRow (i);
		beginRemoveRows (QModelIndex (), startingRow, startingRow + rows - 1);
	}
	Models_.erase (i);
	if (wouldRemove)
		endRemoveRows ();
}

size_t MergeModel::Size () const
{
	return Models_.size ();
}

int MergeModel::GetStartingRow (MergeModel::const_iterator it) const
{
	int result = 0;
	for (models_t::const_iterator i = Models_.begin (); i != it; ++i)
		result += RowCount (*i);
	return result;
}

MergeModel::const_iterator MergeModel::GetModelForRow (int row) const
{
	int counter = 0;
	for (models_t::const_iterator i = Models_.begin (),
			end = Models_.end (); i != end; ++i)
	{
		counter += RowCount (*i);
		if (counter > row)
			return i;
	}
	QString msg = Q_FUNC_INFO;
	msg += ": not found ";
	msg += QString::number (row);
	throw std::runtime_error (qPrintable (msg));
}

MergeModel::iterator MergeModel::GetModelForRow (int row)
{
	int counter = 0;
	for (models_t::iterator i = Models_.begin (),
			end = Models_.end (); i != end; ++i)
	{
		counter += RowCount (*i);
		if (counter > row)
			return i;
	}
	QString msg = Q_FUNC_INFO;
	msg += ": not found ";
	msg += QString::number (row);
	throw std::runtime_error (qPrintable (msg));
}

void MergeModel::handleColumnsAboutToBeInserted (const QModelIndex&, int, int)
{
	qWarning () << "model" << sender ()
		<< "called handleColumnsAboutToBeInserted, ignoring it";
	return;
}

void MergeModel::handleColumnsAboutToBeRemoved (const QModelIndex&, int, int)
{
	qWarning () << "model" << sender ()
		<< "called handleColumnsAboutToBeRemoved, ignoring it";
	return;
}

void MergeModel::handleColumnsInserted (const QModelIndex&, int, int)
{
	qWarning () << "model" << sender ()
		<< "called handleColumnsInserted, ignoring it";
	return;
}

void MergeModel::handleColumnsRemoved (const QModelIndex&, int, int)
{
	qWarning () << "model" << sender ()
		<< "called handleColumnsRemoved, ignoring it";
	return;
}

void MergeModel::handleDataChanged (const QModelIndex& topLeft,
		const QModelIndex& bottomRight)
{
	emit dataChanged (mapFromSource (topLeft), mapFromSource (bottomRight));
}

void MergeModel::handleRowsAboutToBeInserted (const QModelIndex& parent,
		int first, int last)
{
	QAbstractItemModel *model = static_cast<QAbstractItemModel*> (sender ());
	int startingRow = GetStartingRow (FindModel (model));
	beginInsertRows (mapFromSource (parent),
			first + startingRow, last + startingRow);
}

void MergeModel::handleRowsAboutToBeRemoved (const QModelIndex& parent,
		int first, int last)
{
	QAbstractItemModel *model = static_cast<QAbstractItemModel*> (sender ());
	int startingRow = GetStartingRow (FindModel (model));
	try
	{
		beginRemoveRows (mapFromSource (parent),
				first + startingRow, last + startingRow);
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< objectName ()
			<< first
			<< last
			<< startingRow;
		throw;
	}
}

void MergeModel::handleRowsInserted (const QModelIndex&, int, int)
{
	endInsertRows ();
}

void MergeModel::handleRowsRemoved (const QModelIndex&, int, int)
{
	endRemoveRows ();
}

bool MergeModel::AcceptsRow (QAbstractItemModel*, int) const
{
	DefaultAcceptsRowImpl_ = true;
	return true;
}

int MergeModel::RowCount (QAbstractItemModel *model) const
{
	if (!model)
		return 0;

	int orig = model->rowCount ();
	if (DefaultAcceptsRowImpl_)
		return orig;

	int result = 0;
	for (int i = 0; i < orig; ++i)
		result += AcceptsRow (model, i) ? 1 : 0;
	return result;
}

