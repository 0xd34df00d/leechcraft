#include "historymodel.h"

using namespace LeechCraft::Util;

HistoryModel::HistoryModel (QObject *parent)
: QAbstractItemModel (parent)
{
	Headers_ << tr ("Filename")
		<< tr ("Path")
		<< tr ("Size")
		<< tr ("Date");
}

int HistoryModel::columnCount (const QModelIndex&) const
{
	return Headers_.size ();
}

Qt::ItemFlags HistoryModel::flags (const QModelIndex&) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex HistoryModel::index (int row, int column, const QModelIndex&) const
{
    if (!hasIndex (row, column))
        return QModelIndex ();

    return createIndex (row, column);
} 

QVariant HistoryModel::headerData (int column, Qt::Orientation orient, int role) const
{
    if (role != Qt::DisplayRole || orient != Qt::Horizontal)
        return QVariant ();

    return Headers_.at (column);
}

QModelIndex HistoryModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

HistoryModel::~HistoryModel ()
{
}

