/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "subscriptionsmodel.h"
#include <QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QtDebug>
#include <util/sys/paths.h>

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	namespace
	{
		QStringList MakeHeaders ()
		{
			return { SubscriptionsModel::tr ("Name"), SubscriptionsModel::tr ("Last updated"), SubscriptionsModel::tr ("URL") };
		}
	}

	SubscriptionsModel::SubscriptionsModel (QObject *parent)
	: QAbstractItemModel { parent }
	, HeaderLabels_ { MakeHeaders () }
	{
	}

	int SubscriptionsModel::columnCount (const QModelIndex&) const
	{
		return HeaderLabels_.size ();
	}

	QVariant SubscriptionsModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () ||
				role != Qt::DisplayRole)
			return QVariant ();

		int row = index.row ();
		switch (index.column ())
		{
		case 0:
			return Filters_.at (row).SD_.Name_;
		case 1:
			return Filters_.at (row).SD_.LastDateTime_;
		case 2:
			return Filters_.at (row).SD_.URL_.toString ();
		default:
			return QVariant ();
		}
	}

	QVariant SubscriptionsModel::headerData (int section, Qt::Orientation orient, int role) const
	{
		if (orient != Qt::Horizontal ||
				role != Qt::DisplayRole)
			return {};

		return HeaderLabels_.at (section);
	}

	QModelIndex SubscriptionsModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex SubscriptionsModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int SubscriptionsModel::rowCount (const QModelIndex& index) const
	{
		return index.isValid () ? 0 : Filters_.size ();
	}

	namespace
	{
		QList<Filter>::iterator FindFilename (QList<Filter>& filters, const QString& filename)
		{
			return std::find_if (filters.begin (), filters.end (),
					[&filename] (const Filter& other) { return other.SD_.Filename_ == filename; });
		}
	}

	void SubscriptionsModel::SetInitialFilters (const QList<Filter>& filters)
	{
		qDebug () << Q_FUNC_INFO << "adding" << filters.size () << "filters";
		for (const auto& f : filters)
			AddImpl (f);

		LoadSettings ();

		emit filtersListChanged ();
	}

	void SubscriptionsModel::AddFilter (const Filter& filter)
	{
		AddImpl (filter);

		SaveSettings ();

		emit filtersListChanged ();
	}

	void SubscriptionsModel::SetSubData (const SubscriptionData& sd)
	{
		if (!AssignSD (sd))
			qWarning () << Q_FUNC_INFO
				<< "could not find filter for name"
				<< sd.Filename_;
		else
			SaveSettings ();
	}

	void SubscriptionsModel::RemoveFilter (const QString& filename)
	{
		const auto pos = FindFilename (Filters_, filename);
		if (pos == Filters_.end ())
			return;

		RemoveFilter (std::distance (Filters_.begin (), pos));
	}

	void SubscriptionsModel::RemoveFilter (const QModelIndex& index)
	{
		RemoveFilter (index.row ());
	}

	const QList<Filter>& SubscriptionsModel::GetAllFilters () const
	{
		return Filters_;
	}

	void SubscriptionsModel::AddImpl (const Filter& filter)
	{
		const auto pos = FindFilename (Filters_, filter.SD_.Filename_);
		if (pos != Filters_.end ())
		{
			int row = std::distance (Filters_.begin (), pos);
			*pos = filter;
			emit dataChanged (index (row, 0), index (row, columnCount () - 1));
		}
		else
		{
			beginInsertRows ({}, Filters_.size (), Filters_.size ());
			Filters_ << filter;
			endInsertRows ();
		}
	}

	void SubscriptionsModel::RemoveFilter (int pos)
	{
		const auto& filename = Filters_.at (pos).SD_.Filename_;
		auto path = Util::CreateIfNotExists ("cleanweb");
		if (!path.exists (filename))
			qWarning () << Q_FUNC_INFO
					<< "no file"
					<< filename
					<< "in"
					<< path.path ();
		else if (!path.remove (filename))
			qWarning () << Q_FUNC_INFO
					<< "unable to remove"
					<< filename
					<< "in"
					<< path.path ();

		beginRemoveRows ({}, pos, pos);
		Filters_.removeAt (pos);
		endRemoveRows ();

		SaveSettings ();

		emit filtersListChanged ();
	}

	void SubscriptionsModel::SaveSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb");
		settings.beginWriteArray ("Subscriptions");
		settings.remove ("");

		int i = 0;
		for (const auto& f : Filters_)
		{
			settings.setArrayIndex (i++);
			settings.setValue ("URL", f.SD_.URL_);
			settings.setValue ("name", f.SD_.Name_);
			settings.setValue ("fileName", f.SD_.Filename_);
			settings.setValue ("lastDateTime", f.SD_.LastDateTime_);
		}

		settings.endArray ();
	}

	void SubscriptionsModel::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CleanWeb");
		int size = settings.beginReadArray ("Subscriptions");

		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			SubscriptionData sd
			{
				settings.value ("URL").toUrl (),
				settings.value ("name").toString (),
				settings.value ("fileName").toString (),
				settings.value ("lastDateTime").toDateTime ()
			};
			if (!AssignSD (sd))
				qWarning () << Q_FUNC_INFO
					<< "could not find filter for name"
					<< sd.Filename_;
		}

		settings.endArray ();
	}

	bool SubscriptionsModel::AssignSD (const SubscriptionData& sd)
	{
		const auto pos = FindFilename (Filters_, sd.Filename_);
		if (pos == Filters_.end ())
			return false;

		pos->SD_ = sd;
		int row = std::distance (Filters_.begin (), pos);
		emit dataChanged (index (row, 0), index (row, columnCount () - 1));
		return true;
	}
}
}
}
