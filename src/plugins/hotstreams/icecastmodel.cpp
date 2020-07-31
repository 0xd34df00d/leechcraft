/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "icecastmodel.h"
#include <QUrl>
#include <QtDebug>
#include <util/sll/unreachable.h>
#include "roles.h"

Q_DECLARE_METATYPE (QList<QUrl>)

namespace LC
{
namespace HotStreams
{
	bool operator== (const IcecastModel::StationInfo& s1, const IcecastModel::StationInfo& s2)
	{
		return s1.Bitrate_ == s2.Bitrate_ &&
				s1.Name_ == s2.Name_ &&
				s1.Genre_ == s2.Genre_ &&
				s1.MIME_ == s2.MIME_ &&
				s1.URLs_ == s2.URLs_;
	}

	namespace
	{
		enum class IndexType
		{
			Root,
			Category,
			Genre,
			Station
		};

		IndexType GetIndexType (const QModelIndex& index)
		{
			if (!index.isValid ())
				return IndexType::Root;

			const auto id = static_cast<quint32> (index.internalId ());
			if (id == 0xffffffff)
				return IndexType::Category;

			if (!id)
				return IndexType::Genre;

			return IndexType::Station;
		}

		int GetGenreIndex (const QModelIndex& index)
		{
			return index.internalId () - 1;
		}

		quint32 MakeStationId (int genre)
		{
			return genre + 1;
		}

		quint32 MakeGenreId ()
		{
			return 0;
		}

		quint32 MakeCategoryId ()
		{
			return 0xffffffff;
		}
	}

	QModelIndex IcecastModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		switch (GetIndexType (parent))
		{
		case IndexType::Root:
			return createIndex (row, column, MakeCategoryId ());
		case IndexType::Category:
			return createIndex (row, column, MakeGenreId ());
		case IndexType::Genre:
			return createIndex (row, column, MakeStationId (parent.row ()));
		case IndexType::Station:
			return {};
		}

		Util::Unreachable ();
	}

	QModelIndex IcecastModel::parent (const QModelIndex& child) const
	{
		switch (GetIndexType (child))
		{
		case IndexType::Root:
		case IndexType::Category:
			return {};
		case IndexType::Genre:
			return createIndex (0, 0, MakeCategoryId ());
		case IndexType::Station:
			return createIndex (GetGenreIndex (child), 0, MakeGenreId ());
		}

		Util::Unreachable ();
	}

	int IcecastModel::rowCount (const QModelIndex& parent) const
	{
		switch (GetIndexType (parent))
		{
		case IndexType::Root:
			return 1;
		case IndexType::Category:
			return Stations_.size ();
		case IndexType::Genre:
			return Stations_.value (parent.row ()).second.size ();
		case IndexType::Station:
			return 0;
		}

		Util::Unreachable ();
	}

	int IcecastModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	Qt::ItemFlags IcecastModel::flags (const QModelIndex& index) const
	{
		auto flags = QAbstractItemModel::flags (index);
		if (GetIndexType (index) == IndexType::Station)
			flags |= Qt::ItemIsDragEnabled;
		return flags;
	}

	QVariant IcecastModel::data (const QModelIndex& index, int role) const
	{
		if (role == Qt::DecorationRole)
			return RadioIcon_;

		switch (GetIndexType (index))
		{
		case IndexType::Root:
			return {};
		case IndexType::Category:
			switch (role)
			{
			case Qt::DisplayRole:
				return "Icecast";
			case Media::RadioItemRole::ItemType:
				return Media::RadioType::None;
			default:
				return {};
			}
		case IndexType::Genre:
			switch (role)
			{
			case Qt::DisplayRole:
				return Stations_.value (index.row ()).first;
			case Media::RadioItemRole::ItemType:
				return Media::RadioType::None;
			default:
				return {};
			}
		case IndexType::Station:
			return GetStationData (index, role);
		}

		Util::Unreachable ();
	}

	void IcecastModel::SetStations (const StationInfoList_t& stations)
	{
		if (stations == Stations_)
			return;

		const auto prevSize = Stations_.size ();
		if (prevSize)
			beginRemoveRows (index (0, 0, {}), 0, prevSize - 1);
		Stations_.clear ();
		if (prevSize)
			endRemoveRows ();

		const auto newSize = stations.size ();
		if (newSize)
			beginInsertRows (index (0, 0, {}), 0, newSize - 1);
		Stations_ = stations;
		if (newSize)
			endInsertRows ();
	}

	QVariant IcecastModel::GetStationData (const QModelIndex& index, int role) const
	{
		const auto& genred = Stations_.value (GetGenreIndex (index));
		const auto& station = genred.second.value (index.row ());

		switch (role)
		{
		case Qt::ToolTipRole:
			return tr ("Genre: %1\nBitrate: %2 kbps\nType: %3")
					.arg (station.Genre_)
					.arg (station.Bitrate_)
					.arg (station.MIME_);
		case Qt::DisplayRole:
		case StreamItemRoles::PristineName:
		case Media::RadioItemRole::RadioID:
			return station.Name_;
		case StreamItemRoles::PlaylistFormat:
			return "urllist";
		case StreamItemRoles::UrlList:
			return QVariant::fromValue (station.URLs_);
		case Media::RadioItemRole::ItemType:
			return Media::RadioType::Predefined;
		default:
			return {};
		}
	}
}
}
