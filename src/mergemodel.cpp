#include <algorithm>
#include "mergemodel.h"

MergeModel::MergeModel (QObject *parent)
: QAbstractProxyModel (parent)
{
}

MergeModel::~MergeModel ()
{
}

QModelIndex MergeModel::mapFromSource (const QModelIndex& sourceIndex) const
{
}

QModelIndex MergeModel::mapToSource (const QModelIndex& proxyIndex) const
{
}

void MergeModel::setSourceModel (QAbstractItemModel*)
{
	QAbstractProxyModel::setSourceModel (0);
}

void MergeModel::AddModel (QAbstractItemModel *model)
{
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

void MergeModel::RemoveModel (QAbstractItemModel *model)
{
	models_t::iterator i = std::find (Models_.begin (), Models_.end (),
			model);

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
		result += (*it)->rowCount (QModelIndex ());
	return result;
}

