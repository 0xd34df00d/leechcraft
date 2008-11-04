#include "historymodel.h"
#include <iterator>
#include <algorithm>
#include <QSettings>
#include <QtDebug>
#include <plugininterface/proxy.h>

HistoryModel::HistoryModel (QObject *parent)
: LeechCraft::HistoryModel (parent)
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Torrent");
	int max = settings.beginReadArray ("History");
	for (int i = 0; i < max; ++i)
	{
		settings.setArrayIndex (i);
		HistoryItem item;
		item.Name_ = settings.value ("Name").toString ();
		item.Where_ = settings.value ("Where").toString ();
		item.TorrentSize_ = settings.value ("TorrentSize").value<quint64> ();
		item.DateTime_ = settings.value ("Date").toDateTime ();
		item.Tags_ = settings.value ("Tags").toStringList ();
		Items_.push_back (item);
	}
	settings.endArray ();
}

HistoryModel::~HistoryModel ()
{
	SaveSettings ();
}

QVariant HistoryModel::data (const QModelIndex& index, int role) const
{
	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column ())
			{
				case 0:
					return Items_ [index.row ()].Name_;
				case 1:
					return Items_ [index.row ()].Where_;
				case 2:
					return Proxy::Instance ()->MakePrettySize (Items_ [index.row ()].TorrentSize_);
				case 3:
					return Items_ [index.row ()].DateTime_.toString ();
				default:
					return "Unknown field";
			}
		case TagsRole:
			return Items_ [index.row ()].Tags_;
		default:
			return QVariant ();
	}
}

int HistoryModel::rowCount (const QModelIndex& parent) const
{
	return parent.isValid () ? 0 : Items_.size ();
}

void HistoryModel::RemoveItem (const QModelIndex& index)
{
	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Items_.erase (Items_.begin () + index.row ());
	endRemoveRows ();

	SaveSettings ();
}

bool operator== (const HistoryModel::HistoryItem& hi1,
		const HistoryModel::HistoryItem& hi2)
{
	return hi1.Name_ == hi2.Name_ &&
		hi1.Where_ == hi2.Where_;
}

void HistoryModel::AddItem (const HistoryModel::HistoryItem& item)
{
	if (std::find (Items_.begin (), Items_.end (), item) != Items_.end ())
		return;

	beginInsertRows (QModelIndex (), Items_.size (), Items_.size ());
	Items_.push_back (item);
	endInsertRows ();

	SaveSettings ();
}

void HistoryModel::SaveSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Torrent");
    settings.beginWriteArray ("History");
    for (items_t::const_iterator i = Items_.begin (),
			begin = Items_.begin (), end = Items_.end ();
			i != end; ++i)
    {
        settings.setArrayIndex (std::distance (begin, i));
        settings.setValue ("Name", i->Name_);
        settings.setValue ("Where", i->Where_);
        settings.setValue ("TorrentSize", i->TorrentSize_);
        settings.setValue ("Date", i->DateTime_);
		settings.setValue ("Tags", i->Tags_);
    }
    settings.endArray ();
}

