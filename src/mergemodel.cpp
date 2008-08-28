#include <algorithm>
#include <QtDebug>
#include "mergemodel.h"

MergeModel::MergeModel (QObject *parent)
: QAbstractProxyModel (parent)
{
	Headers_ << tr ("Name")
		<< tr ("State")
		<< tr ("Progress")
		<< tr ("Speed");
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
	if (orient != Qt::Horizontal && role != Qt::DisplayRole)
		return QVariant ();

	return Headers_.at (column);
}

QVariant MergeModel::data (const QModelIndex& index, int role) const
{
	if (index.isValid ())
	{
		QModelIndex mapped = mapToSource (index);
		return mapped.model ()->data (mapped, role);
	}
	else
	{
		int indexRow = index.row ();
		const_iterator modIter = GetModelForRow (indexRow);
		int startingRow = GetStartingRow (modIter);
		return (*modIter)->data (createIndex (indexRow - startingRow, index.column (), role));
	}
}

QModelIndex MergeModel::index (int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid ())
	{
		QModelIndex mapped = mapToSource (parent);
		return mapped.model ()->index (row, column, mapped);
	}
	else
		return createIndex (row, column);
}

QModelIndex MergeModel::parent (const QModelIndex& index) const
{
	if (!index.isValid ())
		return QModelIndex ();

	QModelIndex mapped = mapToSource (index);
	return mapped.model ()->parent (mapped);
}

int MergeModel::rowCount (const QModelIndex& parent) const
{
	if (!parent.isValid ())
	{
		int result = 0;
		for (models_t::const_iterator i = Models_.begin (),
				end = Models_.end ();
				i != end; ++i)
			result += (*i)->rowCount ();
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
	QModelIndex mappedParent = mapToSource (proxyIndex.parent ());
	const_iterator modIter = GetModelForRow (proxyRow);
	int startingRow = mappedParent.isValid () ? 0 : GetStartingRow (modIter);
	return (*modIter)->index (proxyRow - startingRow, proxyColumn, mappedParent);
}

void MergeModel::setSourceModel (QAbstractItemModel*)
{
	qWarning () << Q_FUNC_INFO << "never call this";
	QAbstractProxyModel::setSourceModel (0);
}

void MergeModel::AddModel (QAbstractItemModel *model)
{
	if (!model)
		return;

	int rows = model->rowCount (QModelIndex ());
	bool wouldInsert = false;
	if (rows > 0)
		wouldInsert = true;

	if (wouldInsert)
		beginInsertRows (QModelIndex (), rowCount (), rowCount () + rows - 1);
	Models_.push_back (model);
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
		return;

	int rows = model->rowCount ();
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

int MergeModel::GetStartingRow (MergeModel::const_iterator it) const
{
	int result = 0;
	for (models_t::const_iterator i = Models_.begin (); i != it; ++i)
		result += (*i)->rowCount (QModelIndex ());
	return result;
}

MergeModel::const_iterator MergeModel::GetModelForRow (int row) const
{
	int counter = 0;
	for (models_t::const_iterator i = Models_.begin (),
			end = Models_.end (); i != end; ++i)
	{
		counter += (*i)->rowCount (QModelIndex ());
		if (counter > row)
			return i;
	}
	qWarning () << Q_FUNC_INFO << "not found";
	return Models_.end ();
}

MergeModel::iterator MergeModel::GetModelForRow (int row)
{
	int counter = 0;
	for (models_t::iterator i = Models_.begin (),
			end = Models_.end (); i != end; ++i)
	{
		counter += (*i)->rowCount (QModelIndex ());
		if (counter > row)
			return i;
	}
	qWarning () << Q_FUNC_INFO << "not found";
	return Models_.end ();
}

