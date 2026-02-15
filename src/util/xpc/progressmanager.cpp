/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progressmanager.h"

namespace LC::Util
{
	ProgressModelRow::ProgressModelRow (ProgressManager& model, const QModelIndex& index)
	: Manager_ { model }
	, Index_ { index }
	{
	}

	ProgressModelRow::~ProgressModelRow ()
	{
		if (Index_.isValid ())
			Manager_.Model_.RemoveItem (Index_.row ());
	}

	void ProgressModelRow::Disengage ()
	{
		Index_ = QPersistentModelIndex {};
	}

	void ProgressModelRow::SetDone (qint64 done)
	{
		if (Index_.isValid ())
			Manager_.Model_.SetField<&ProgressManager::Item::Done_> (Index_.row (), done);
	}

	void ProgressModelRow::SetTotal (qint64 total)
	{
		if (Index_.isValid ())
			Manager_.Model_.SetField<&ProgressManager::Item::Total_> (Index_.row (), total);
	}

	void ProgressModelRow::ChangeTotalBy (qint64 delta)
	{
		if (Index_.isValid ())
		{
			const auto total = Manager_.Model_.GetItems () [Index_.row ()].Total_;
			Manager_.Model_.SetField<&ProgressManager::Item::Total_> (Index_.row (), total + delta);
		}
	}

	void ProgressModelRow::SetState (ProcessState state, QString customText)
	{
		if (Index_.isValid ())
			Manager_.Model_.SetFields<
					&ProgressManager::Item::State_,
					&ProgressManager::Item::CustomStateText_
				> (Index_.row (), state, std::move (customText));
	}

	void ProgressModelRow::SetCustomData (const QVariant& data)
	{
		if (Index_.isValid ())
			Manager_.Model_.GetMutItems () [Index_.row ()].CustomData_ = data;
	}

	void ProgressModelRow::operator++ ()
	{
		if (!Index_.isValid ())
			return;

		const auto done = Manager_.Model_.GetItems () [Index_.row ()].Done_;
		Manager_.Model_.SetField<&ProgressManager::Item::Done_> (Index_.row (), done + 1);
	}

	ProgressManager::ProgressManager (QObject *parent)
	: QObject { parent }
	{
	}

	void ProgressManager::SetGlobalData (const QVariant& data, int role)
	{
		Model_.SetGlobalData (data, role);
	}

	QAbstractItemModel& ProgressManager::GetModel ()
	{
		return Model_;
	}

	std::unique_ptr<ProgressModelRow> ProgressManager::AddRow (RowInfo info)
	{
		return AddRow (std::move (info), {});
	}

	std::unique_ptr<ProgressModelRow> ProgressManager::AddRow (RowInfo info, Initializers inits)
	{
		auto name = info.Name_;
		const auto pos = Model_.AddItem ({
				.RowInfo_ = std::move (info),
				.Total_ = inits.Total_,
				.State_ = inits.State_,
				.CustomStateText_ = std::move (inits.CustomStateText_),
				.Icon_ = std::move (inits.Icon_),

				.CachedName_ = std::move (name),

				.CustomData_ = std::move (inits.CustomData_),
			});
		return std::make_unique<ProgressModelRow> (*this, Model_.index (pos, 0));
	}

	QVariant ProgressManager::GetCustomData (const QModelIndex& index) const
	{
		return Model_.GetItems () [index.row ()].CustomData_;
	}
}
