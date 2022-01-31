/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectionsortermodel.h"
#include <util/sll/statichash.h>
#include "localcollectionmodel.h"
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		enum CompareFlag
		{
			None = 0x0,
			WithoutThe = 0x1
		};
		Q_DECLARE_FLAGS (CompareFlags, CompareFlag);

		bool IntCompare (const QVariant& left, const QVariant& right, CompareFlags)
		{
			return left.toInt () < right.toInt ();
		}

		bool StrCompare (const QVariant& left, const QVariant& right, CompareFlags)
		{
			return QString::localeAwareCompare (left.toString (), right.toString ()) < 0;
		}

		bool NameCompare (const QVariant& left, const QVariant& right, CompareFlags flags)
		{
			return CompareArtists (left.toString (), right.toString (), flags & WithoutThe);
		}

		using Comparator_t = bool (*) (const QVariant&, const QVariant&, CompareFlags);

		using Util::KVPair;
		constexpr auto RoleComparators = Util::MakeHash<LocalCollectionModel::Role, Comparator_t> (
				KVPair { LocalCollectionModel::Role::ArtistName, NameCompare },
				KVPair { LocalCollectionModel::Role::AlbumName, StrCompare },
				KVPair { LocalCollectionModel::Role::AlbumYear, IntCompare },
				KVPair { LocalCollectionModel::Role::TrackNumber, IntCompare },
				KVPair { LocalCollectionModel::Role::TrackTitle, StrCompare },
				KVPair { LocalCollectionModel::Role::TrackPath, StrCompare }
			);

		bool RoleCompare (const QModelIndex& left, const QModelIndex& right,
				std::initializer_list<LocalCollectionModel::Role> roles, CompareFlags flags)
		{
			for (auto role : roles)
			{
				const auto& lData = left.data (role);
				const auto& rData = right.data (role);
				if (lData != rData)
					return RoleComparators (role) (lData, rData, flags);
			}
			return false;
		}
	}

	CollectionSorterModel::CollectionSorterModel (QObject *parent)
	: QSortFilterProxyModel { parent }
	{
		XmlSettingsManager::Instance ().RegisterObject ("SortWithThe",
				this,
				[this] (const QVariant& var)
				{
					UseThe_ = var.toBool ();
					invalidate ();
				});
	}

	bool CollectionSorterModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		CompareFlags flags = CompareFlag::None;
		if (!UseThe_)
			flags |= CompareFlag::WithoutThe;

		using enum LocalCollectionModel::Role;

		switch (left.data (LocalCollectionModel::Role::Node).toInt ())
		{
		case LocalCollectionModel::NodeType::Artist:
			return RoleCompare (left, right, { ArtistName }, flags);
		case LocalCollectionModel::NodeType::Album:
			return RoleCompare (left, right, { AlbumYear, AlbumName }, flags);
		case LocalCollectionModel::NodeType::Track:
			return RoleCompare (left, right, { TrackNumber, TrackTitle, TrackPath }, flags);
		default:
			return QSortFilterProxyModel::lessThan (left, right);
		}
	}
}
}
