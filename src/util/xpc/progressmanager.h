/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIcon>
#include <util/models/itemsmodel.h>
#include <interfaces/ijobholder.h>
#include "xpcconfig.h"

namespace LC::Util
{
	class ProgressManager;

	class UTIL_XPC_API ProgressModelRow final
	{
		ProgressManager& Manager_;
		QPersistentModelIndex Index_;
	public:
		explicit ProgressModelRow (ProgressManager&, const QModelIndex&);
		~ProgressModelRow ();

		void Disengage ();

		void SetDone (qint64);
		void SetTotal (qint64);
		void ChangeTotalBy (qint64 delta);
		void SetState (ProcessState state, QString customText = {});

		void SetCustomData (const QVariant&);

		void operator++ ();
	};

	class UTIL_XPC_API ProgressManager : public QObject
	{
		friend class ProgressModelRow;

		struct Item
		{
			RoleOf<RowInfo, +JobHolderRole::RowInfo> RowInfo_;
			RoleOf<qint64, +JobHolderProcessRole::Done> Done_ = 0;
			RoleOf<qint64, +JobHolderProcessRole::Total> Total_ = 0;
			RoleOf<ProcessState, +JobHolderProcessRole::State> State_ = ProcessState::Running;
			RoleOf<QString, +JobHolderProcessRole::StateCustomText> CustomStateText_ {};
			RoleOf<QIcon, Qt::DecorationRole> Icon_;

			RoleOf<QString, Qt::DisplayRole> CachedName_;

			QVariant CustomData_ {};
		};
		RoledItemsModel<Item> Model_;
	public:
		constexpr static auto MaxRole = MaxValue<JobHolderProcessRole>;

		explicit ProgressManager (QObject *parent = nullptr);

		void SetGlobalData (const QVariant& data, int role);

		QAbstractItemModel& GetModel ();
		IJobHolderRepresentationHandler_ptr CreateDefaultHandler ();

		struct Initializers
		{
			ProcessState State_ = ProcessState::Running;
			qint64 Total_ = 0;
			QString CustomStateText_ {};
			QIcon Icon_ {};
			QVariant CustomData_ {};
		};

		[[nodiscard]] std::unique_ptr<ProgressModelRow> AddRow (RowInfo);
		[[nodiscard]] std::unique_ptr<ProgressModelRow> AddRow (RowInfo, Initializers);

		QVariant GetCustomData (const QModelIndex&) const;
	};
}
