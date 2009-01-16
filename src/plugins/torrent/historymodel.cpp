#include "historymodel.h"
#include <iterator>
#include <algorithm>
#include <QSettings>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>

using LeechCraft::Util::Proxy;

bool HistoryModel::HistoryItem::operator== (const HistoryModel::HistoryItem& other) const
{
	return Name_ == other.Name_ &&
		TorrentSize_ == other.TorrentSize_ &&
		Where_ == other.Where_;
}

HistoryModel::HistoryModel (QObject *parent)
: LeechCraft::Util::HistoryModel (parent)
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
				case HFilename:
					return Items_ [index.row ()].Name_;
				case HPath:
					return Items_ [index.row ()].Where_;
				case HSize:
					return Proxy::Instance ()->MakePrettySize (Items_ [index.row ()].TorrentSize_);
				case HDate:
					return Items_ [index.row ()].DateTime_.toString ();
				case HTags:
					return Items_ [index.row ()].Tags_.join (" ");
				default:
					return "Unknown field";
			}
		case LeechCraft::TagsRole:
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
	settings.remove ("");
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

