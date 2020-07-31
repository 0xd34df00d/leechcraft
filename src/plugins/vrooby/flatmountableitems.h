/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QAbstractItemModel>
#include <util/models/flattenfiltermodel.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/devices/deviceroles.h>

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Vrooby
{
	class FlatMountableItems : public Util::RoleNamesMixin<Util::FlattenFilterModel>
	{
		Q_OBJECT

		QList<QPersistentModelIndex> SourceIndexes_;
	public:
		enum CustomRoles
		{
			FormattedTotalSize = MassStorageRole::MassStorageRoleMax + 1,
			FormattedFreeSpace,
			UsedPercentage,
			MountButtonIcon,
			ToggleHiddenIcon,
			MountedAt
		};

		FlatMountableItems (QObject* = 0);

		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
	protected:
		bool IsIndexAccepted (const QModelIndex&) const;
	};
}
}
