/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QIcon>
#include <QAbstractNativeEventFilter>
#include <util/models/rolenamesmixin.h>

namespace LC
{
namespace Mellonetray
{
	class TrayModel : public Util::RoleNamesMixin<QAbstractItemModel>
					, public QAbstractNativeEventFilter
	{
		Q_OBJECT

		bool IsValid_ = false;

		ulong TrayWinID_ = 0;
		int DamageEvent_ = 0;

		struct TrayItem
		{
			ulong WID_;
		};
		QList<TrayItem> Items_;

		enum Role
		{
			ItemID = Qt::UserRole + 1
		};

		TrayModel ();

		TrayModel (const TrayModel&) = delete;
		TrayModel (TrayModel&&) = delete;

		TrayModel& operator= (const TrayModel&) = delete;
		TrayModel& operator= (TrayModel&&) = delete;
	public:
		static TrayModel& Instance ();
		void Release ();

		bool IsValid () const;

		int columnCount (const QModelIndex& parent = QModelIndex()) const override;
		int rowCount (const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex& child) const override;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;

		bool nativeEventFilter (const QByteArray&, void*, qintptr*) override;
	private:
		template<typename T>
		void HandleClientMsg (T);

		void Add (ulong);
		void Remove (ulong);
		void Update (ulong);

		QList<TrayItem>::iterator FindItem (ulong);
	signals:
		void updateRequired (ulong);
	};
}
}
