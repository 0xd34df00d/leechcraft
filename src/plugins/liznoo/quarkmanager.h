/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LC
{
namespace Liznoo
{
	struct BatteryInfo;

	class QuarkManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
		QHash<QString, QStandardItem*> Battery2Item_;

		Q_PROPERTY (QObject* batteryModel READ GetBatteryModel NOTIFY batteryModelChanged)
	public:
		QuarkManager (QObject* = nullptr);

		QObject* GetBatteryModel () const;
	public slots:
		void handleBatteryInfo (const Liznoo::BatteryInfo&);
	signals:
		void batteryModelChanged ();

		void batteryHistoryDialogRequested (const QString&);
	};
}
}
