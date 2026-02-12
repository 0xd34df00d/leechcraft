/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QIcon>
#include <util/models/itemsmodel.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/isyncplugin.h>
#include "mtp.h"
#include "interfaces/devices/deviceroles.h"

class QAbstractItemModel;
class QSortFilterProxyModel;
class QModelIndex;

namespace LC::LMP::MTPSync
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public ILMPPlugin
				 , public ISyncPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::LMP::ILMPPlugin
				LC::LMP::ISyncPlugin
				)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.MTPSync")

		std::unique_ptr<MtpRunner> Mtp_;

		struct ModelRow
		{
			Util::RoleOf<QByteArray, CommonDevRole::DevPersistentID> Serial_;
			Util::RoleOf<QString, Qt::DisplayRole> DevName_;
			Util::RoleOf<QIcon, Qt::DecorationRole> Icon_;

			QList<DeviceStorage> Storages_;
		};
		Util::RoledItemsModel<ModelRow> *DevicesModel_;

		ILMPProxy_ptr LMPProxy_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		void SetLMPProxy (ILMPProxy_ptr) override;

		QObject* GetQObject () override;
		QString GetSyncSystemName () const override;
		QAbstractItemModel& GetSyncTargetsModel () override;
		ISyncPluginConfigWidget_ptr MakeConfigWidget (const QModelIndex&) override;
		void RefreshSyncTargets () override;
		Util::ContextTask<UploadResult> Upload (UploadJob) override;
	private:
		void AddDevices (const QList<MtpDeviceInfo>&);
	};
}
