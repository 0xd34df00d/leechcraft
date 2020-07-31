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
#include <interfaces/devices/deviceroles.h>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace LMP
{
	class IUnmountableSync;
	struct UnmountableDevInfo;

	class UnmountableDevManager : public QObject
	{
		Q_OBJECT

		QObjectList Managers_;

		QStandardItemModel * const DevListModel_;
		QHash<QObject*, QHash<QByteArray, QStandardItem*>> Items_;

		enum Roles
		{
			DeviceInfo = CommonDevRole::CommonDevRoleMax + 1,
			ManagerObj
		};
	public:
		UnmountableDevManager (QObject* = 0);

		QAbstractItemModel* GetDevListModel () const;
		void InitializePlugins ();

		QObject* GetDeviceManager (int) const;
		UnmountableDevInfo GetDeviceInfo (int) const;

		void Refresh ();
	private slots:
		void rebuildAvailableDevices ();
	};
}
}
