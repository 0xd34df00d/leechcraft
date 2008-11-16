#include "favoritesmodel.h"
#include <QSettings>
#include <plugininterface/proxy.h>
#include "filtermodel.h"

FavoritesModel::FavoritesModel (QObject *parent)
: QAbstractItemModel (parent)
{
	ItemHeaders_ << tr ("Title") << tr ("URL") << tr ("Tags");
	LoadData ();
}

FavoritesModel::~FavoritesModel ()
{
}

int FavoritesModel::columnCount (const QModelIndex&) const
{
	return ItemHeaders_.size ();
}

QVariant FavoritesModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column ())
			{
				case 0:
					return Items_ [index.row ()].Title_;
				case 1:
					return Items_ [index.row ()].URL_;
				case 2:
					return Items_ [index.row ()].Tags_.join (" ");
				default:
					return QVariant ();
			}
		case TagsRole:
			return Items_ [index.row ()].Tags_;
		default:
			return QVariant ();
	}
}

Qt::ItemFlags FavoritesModel::flags (const QModelIndex& index) const
{
	Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column () == ColumnTags)
		result |= Qt::ItemIsEditable;
	return result;
}

QVariant FavoritesModel::headerData (int column, Qt::Orientation orient,
		int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return ItemHeaders_.at (column);
    else
        return QVariant ();
}

QModelIndex FavoritesModel::index (int row, int column,
		const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    return createIndex (row, column);
}

QModelIndex FavoritesModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int FavoritesModel::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Items_.size ();
}

bool FavoritesModel::setData (const QModelIndex& index,
		const QVariant& value, int)
{
	if (index.column () != ColumnTags)
		return false;

	Items_ [index.row ()].Tags_ = value.toStringList ();
	emit dataChanged (index, index);
	SaveData ();
	return true;
}

void FavoritesModel::AddItem (const QString& title, const QString& url,
	   const QStringList& tags)
{
	FavoritesItem item = { title, url, tags };

	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	Items_.push_back (item);
	endInsertRows ();

	SaveData ();
}

void FavoritesModel::removeItem (const QModelIndex& index)
{
	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Items_.erase (Items_.begin () + index.row ());
	endRemoveRows ();

	SaveData ();
}

void FavoritesModel::LoadData ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	int size = settings.beginReadArray ("Favorites");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		FavoritesItem item;
		item.Title_ = settings.value ("Title").toString ();
		item.URL_ = settings.value ("URL").toString ();
		item.Tags_ = settings.value ("Tags").toStringList ();
		Items_.push_back (item);
	}
	settings.endArray ();
}

void FavoritesModel::SaveData () const
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	settings.beginWriteArray ("Favorites");
	settings.remove ("");
	for (size_t i = 0; i < Items_.size (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("Title", Items_ [i].Title_);
		settings.setValue ("URL", Items_ [i].URL_);
		settings.setValue ("Tags", Items_ [i].Tags_);
	}
	settings.endArray ();
}

