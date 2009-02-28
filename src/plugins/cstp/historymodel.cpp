#include "historymodel.h"
#include <algorithm>
#include <stdexcept>
#include <QTimer>
#include <QSettings>
#include <QDataStream>
#include <QtDebug>
#include <plugininterface/proxy.h>

using LeechCraft::Util::Proxy;

HistoryModel::HistoryModel (QObject *parent)
: LeechCraft::Util::HistoryModel (parent)
, SaveScheduled_ (false)
{
	ReadSettings ();
}

HistoryModel::~HistoryModel ()
{
	writeSettings ();
}

void HistoryModel::Add (const HistoryModel::Item& item)
{
	beginInsertRows (QModelIndex (), Items_.size (), Items_.size ());
	Items_.push_back (item);
	endInsertRows ();
	ScheduleSave ();
}

void HistoryModel::RemoveItem (const QModelIndex& index)
{
	if (!index.isValid ())
		return;
	int pos = index.row ();
	beginRemoveRows (QModelIndex (), pos, pos);
	Items_.erase (Items_.begin () + pos);
	endRemoveRows ();
	ScheduleSave ();
}

QVariant HistoryModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (role == Qt::DisplayRole)
	{
		Item item = Items_ [index.row ()];
		switch (index.column ())
		{
			case HFilename:
				return item.Filename_;
			case HPath:
				return item.URL_;
			case HDate:
				return item.DateTime_;
			case HTags:
				return QStringList ();
			default:
				return "Unknown field";
		}
	}
	else if (role == HistoryModel::RolePath)
		return Items_ [index.row ()].Filename_;
    else
        return QVariant ();

	return QVariant ();
}

int HistoryModel::rowCount (const QModelIndex& parent) const
{
	return parent.isValid () ? 0 : Items_.size ();
}

namespace
{
	HistoryModel::Item Deserialize (QByteArray& data)
	{
		HistoryModel::Item result;
		QDataStream in (&data, QIODevice::ReadOnly);
		int version = 0;
		in >> version;
		if (version == 1)
		{
			in >> result.Filename_
				>> result.URL_
				>> result.Size_
				>> result.DateTime_;
		}
		else
			throw std::runtime_error ("Unknown version");
		return result;
	}
};

void HistoryModel::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CSTP");
	int size = settings.beginReadArray ("History");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		QByteArray data = settings.value ("Item").toByteArray ();
		HistoryModel::Item item;
		try
		{
			item = Deserialize (data);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			continue;
		}
		Items_.push_back (item);
	}
	settings.endArray ();
}

void HistoryModel::ScheduleSave ()
{
	if (SaveScheduled_)
		return;

	QTimer::singleShot (1000, this, SLOT (writeSettings ()));
	SaveScheduled_ = true;
}

namespace
{
	QByteArray Serialize (const HistoryModel::Item& item)
	{
		QByteArray result;
		{
			QDataStream out (&result, QIODevice::WriteOnly);
			out << 1
				<< item.Filename_
				<< item.URL_
				<< item.Size_
				<< item.DateTime_;
		}
		return result;
	}

	struct WriteOut
	{
		QSettings& Settings_;
		int Index_;

		WriteOut (QSettings& settings)
		: Settings_ (settings)
		, Index_ (0)
		{
		}

		void operator() (const HistoryModel::Item& item)
		{
			Settings_.setArrayIndex (Index_++);
			Settings_.setValue ("Item", Serialize (item));
		}
	};
};

void HistoryModel::writeSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CSTP");
	settings.beginWriteArray ("History");
	settings.remove ("");
	std::for_each (Items_.begin (), Items_.end (),
			WriteOut (settings));
	SaveScheduled_ = false;
	settings.endArray ();
}

