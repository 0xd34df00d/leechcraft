#include "searchhandler.h"

SearchHandler::SearchHandler ()
{
}

int SearchHandler::columnCount (const QModelIndex&) const
{
	return 3;
}

QVariant SearchHandler::data (const QModelIndex& index, int role) const
{
}

Qt::ItemFlags SearchHandler::flags (const QModelIndex& index) const
{
	if (!index.isValid ())
		return 0;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SearchHandler::headerData (int header, Qt::Orientation orient, int role) const
{
	if (orient == Qt::Horizontal && role == Qt::DisplayRole)
		return QString ("");
	else
		return QVariant ();
}

QModelIndex SearchHandler::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex SearchHandler::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int SearchHandler::rowCount (const QModelIndex& parent) const
{
	return 0;
//	return parent.isValid () ? 0 : Results_.size ();
}

void SearchHandler::Start ()
{
}

