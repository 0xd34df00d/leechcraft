/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include <interfaces/devices/deviceroles.h>

namespace LC::Vrooby
{
	class TrayProxyModel : public QSortFilterProxyModel
	{
		QSet<QString> Hidden_;
		bool FilterEnabled_ = true;
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

		explicit TrayProxyModel (QObject *parent);

		QVariant data (const QModelIndex& index, int role) const override;
		QHash<int, QByteArray> roleNames () const override;
		void ToggleHidden (const QString& id);
		int GetHiddenCount () const;

		void ToggleFilter ();
	protected:
		bool filterAcceptsRow (int row, const QModelIndex&) const override;
	};
}
