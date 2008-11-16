#include "historymodel.h"
#include <QSettings>
#include <plugininterface/proxy.h>

HistoryModel::HistoryModel (QObject *parent)
: QAbstractItemModel (parent)
{
	ItemHeaders_ << tr ("Title")
		<< tr ("Date")
		<< tr ("URL");
	LoadData ();
}

HistoryModel::~HistoryModel ()
{
}

int HistoryModel::columnCount (const QModelIndex&) const
{
	return ItemHeaders_.size ();
}

QVariant HistoryModel::data (const QModelIndex& index, int role) const
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
					return Items_ [index.row ()].DateTime_.toString ();
				case 2:
					return Items_ [index.row ()].URL_;
				default:
					return QVariant ();
			}
		default:
			return QVariant ();
	}
}

Qt::ItemFlags HistoryModel::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant HistoryModel::headerData (int column, Qt::Orientation orient,
		int role) const
{
	if (orient == Qt::Horizontal && role == Qt::DisplayRole)
		return ItemHeaders_.at (column);
	else
		return QVariant ();
}

QModelIndex HistoryModel::index (int row, int column,
		const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex HistoryModel::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int HistoryModel::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Items_.size ();
}

void HistoryModel::AddItem (const QString& title, const QString& url,
		const QDateTime& date)
{
	HistoryItem item =
	{
		title,
		date,
		url
	};
	beginInsertRows (QModelIndex (), 0, 0);
	Items_.push_front (item);
	endInsertRows ();

	SaveData ();
}

void HistoryModel::LoadData ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	int size = settings.beginReadArray ("History");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		HistoryItem item =
		{
			settings.value ("Title").toString (),
			settings.value ("Date").toDateTime (),
			settings.value ("URL").toString ()
		};
		Items_.push_back (item);
	}
	settings.endArray ();
}

void HistoryModel::SaveData () const
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	settings.beginWriteArray ("History");
	settings.remove ("");
	for (size_t i = 0; i < Items_.size (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("Title", Items_ [i].Title_);
		settings.setValue ("Date", Items_ [i].DateTime_);
		settings.setValue ("URL", Items_ [i].URL_);
	}
	settings.endArray ();
}

