/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkmanager.h"
#include <QStandardItemModel>
#include <util/models/rolenamesmixin.h>
#include "batteryinfo.h"

namespace LC
{
namespace Liznoo
{
	namespace
	{
		class QuarkModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				BatteryId = Qt::UserRole + 1,
				Percentage,
				IsCharging,
				TimeToEmpty,
				TimeToFull
			};

			QuarkModel (QObject *parent = nullptr)
			: Util::RoleNamesMixin<QStandardItemModel> { parent }
			{
				QHash<int, QByteArray> names;
				names [BatteryId] = "batteryId";
				names [Percentage] = "percentage";
				names [IsCharging] = "isCharging";
				names [TimeToEmpty] = "timeToEmpty";
				names [TimeToFull] = "timeToFull";
				setRoleNames (names);
			}
		};
	}

	QuarkManager::QuarkManager (QObject *parent)
	: QObject { parent }
	, Model_ { new QuarkModel { this } }
	{
	}

	QObject* QuarkManager::GetBatteryModel () const
	{
		return Model_;
	}

	void QuarkManager::handleBatteryInfo (const BatteryInfo& info)
	{
		const auto& id = info.ID_;
		const auto isNew = !Battery2Item_.contains (id);
		const auto item = isNew ?  new QStandardItem : Battery2Item_ [id];

		item->setData (info.ID_, QuarkModel::BatteryId);
		item->setData (info.Percentage_, QuarkModel::Percentage);
		item->setData (info.TimeToFull_ && !info.TimeToEmpty_, QuarkModel::IsCharging);
		item->setData (info.TimeToEmpty_, QuarkModel::TimeToEmpty);
		item->setData (info.TimeToFull_, QuarkModel::TimeToFull);

		if (isNew)
		{
			Model_->appendRow (item);
			Battery2Item_ [id] = item;
		}
	}
}
}
