/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "collectionsortermodel.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		template<typename T>
		bool VarCompare (const QVariant& left, const QVariant& right)
		{
			return left.value<T> () < right.value<T> ();
		}

		template<>
		bool VarCompare<QString> (const QVariant& left, const QVariant& right)
		{
			return QString::localeAwareCompare (left.toString (), right.toString ()) < 0;
		}

		bool NameCompare (const QVariant& left, const QVariant& right)
		{
			auto leftStr = left.toString ();
			auto rightStr = right.toString ();

			auto chopStr = [] (QString& str)
			{
				if (str.startsWith ("the ", Qt::CaseInsensitive))
					str = str.mid (4);
			};

			chopStr (leftStr);
			chopStr (rightStr);

			return QString::localeAwareCompare (leftStr, rightStr) < 0;
		}

		struct Comparators
		{
			typedef std::function<bool (const QVariant&, const QVariant&)> Comparator_t;
			QHash<LocalCollection::Role, Comparator_t> Role2Cmp_;

			Comparators ()
			{
				Role2Cmp_ [LocalCollection::Role::ArtistName] = NameCompare;
				Role2Cmp_ [LocalCollection::Role::AlbumName] = VarCompare<QString>;
				Role2Cmp_ [LocalCollection::Role::AlbumYear] = VarCompare<int>;
				Role2Cmp_ [LocalCollection::Role::TrackNumber] = VarCompare<int>;
				Role2Cmp_ [LocalCollection::Role::TrackTitle] = VarCompare<QString>;
				Role2Cmp_ [LocalCollection::Role::TrackPath] = VarCompare<QString>;
			}
		};

		bool RoleCompare (const QModelIndex& left, const QModelIndex& right,
				QList<LocalCollection::Role> roles)
		{
			static Comparators comparators;
			while (!roles.isEmpty ())
			{
				auto role = roles.takeFirst ();
				const auto& lData = left.data (role);
				const auto& rData = right.data (role);
				if (lData != rData)
					return comparators.Role2Cmp_ [role] (lData, rData);
			}
			return false;
		}
	}
	CollectionSorterModel::CollectionSorterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
	}

	bool CollectionSorterModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		const auto type = left.data (LocalCollection::Role::Node).toInt ();
		QList<LocalCollection::Role> roles;
		switch (type)
		{
		case LocalCollection::NodeType::Artist:
			roles << LocalCollection::Role::ArtistName;
			break;
		case LocalCollection::NodeType::Album:
			roles << LocalCollection::Role::AlbumYear
					<< LocalCollection::Role::AlbumName;
			break;
		case LocalCollection::NodeType::Track:
			roles << LocalCollection::Role::TrackNumber
					<< LocalCollection::Role::TrackTitle
					<< LocalCollection::Role::TrackPath;
			break;
		default:
			return QSortFilterProxyModel::lessThan (left, right);
		}
		return RoleCompare (left, right, roles);
	}
}
}
