/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectionsortermodel.h"
#include <functional>
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

		template<typename T>
		bool VarCompare (const QVariant& left, const QVariant& right, CompareFlags)
		{
			return left.value<T> () < right.value<T> ();
		}

		template<>
		bool VarCompare<QString> (const QVariant& left, const QVariant& right, CompareFlags)
		{
			return QString::localeAwareCompare (left.toString (), right.toString ()) < 0;
		}

		bool NameCompare (const QVariant& left, const QVariant& right, CompareFlags flags)
		{
			return CompareArtists (left.toString (), right.toString (), flags & WithoutThe);
		}

		struct Comparators
		{
			typedef std::function<bool (const QVariant&, const QVariant&, CompareFlags)> Comparator_t;
			QHash<LocalCollectionModel::Role, Comparator_t> Role2Cmp_;

			Comparators ()
			{
				Role2Cmp_ [LocalCollectionModel::Role::ArtistName] = NameCompare;
				Role2Cmp_ [LocalCollectionModel::Role::AlbumName] = VarCompare<QString>;
				Role2Cmp_ [LocalCollectionModel::Role::AlbumYear] = VarCompare<int>;
				Role2Cmp_ [LocalCollectionModel::Role::TrackNumber] = VarCompare<int>;
				Role2Cmp_ [LocalCollectionModel::Role::TrackTitle] = VarCompare<QString>;
				Role2Cmp_ [LocalCollectionModel::Role::TrackPath] = VarCompare<QString>;
			}
		};

		bool RoleCompare (const QModelIndex& left, const QModelIndex& right,
				QList<LocalCollectionModel::Role> roles, CompareFlags flags)
		{
			static Comparators comparators;
			while (!roles.isEmpty ())
			{
				auto role = roles.takeFirst ();
				const auto& lData = left.data (role);
				const auto& rData = right.data (role);
				if (lData != rData)
					return comparators.Role2Cmp_ [role] (lData, rData, flags);
			}
			return false;
		}
	}

	CollectionSorterModel::CollectionSorterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, UseThe_ (true)
	{
		XmlSettingsManager::Instance ().RegisterObject ("SortWithThe",
				this,
				"handleUseTheChanged");

		handleUseTheChanged ();
	}

	bool CollectionSorterModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		const auto type = left.data (LocalCollectionModel::Role::Node).toInt ();
		QList<LocalCollectionModel::Role> roles;
		switch (type)
		{
		case LocalCollectionModel::NodeType::Artist:
			roles << LocalCollectionModel::Role::ArtistName;
			break;
		case LocalCollectionModel::NodeType::Album:
			roles << LocalCollectionModel::Role::AlbumYear
					<< LocalCollectionModel::Role::AlbumName;
			break;
		case LocalCollectionModel::NodeType::Track:
			roles << LocalCollectionModel::Role::TrackNumber
					<< LocalCollectionModel::Role::TrackTitle
					<< LocalCollectionModel::Role::TrackPath;
			break;
		default:
			return QSortFilterProxyModel::lessThan (left, right);
		}

		CompareFlags flags = CompareFlag::None;
		if (!UseThe_)
			flags |= CompareFlag::WithoutThe;

		return RoleCompare (left, right, roles, flags);
	}

	void CollectionSorterModel::handleUseTheChanged ()
	{
		UseThe_ = XmlSettingsManager::Instance ()
				.property ("SortWithThe").toBool ();
		invalidate ();
	}
}
}
