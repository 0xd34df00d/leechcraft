/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QAction>
#include <interfaces/iinfo.h>
#include <interfaces/devices/iremovabledevmanager.h>
#include <interfaces/iactionsexporter.h>

namespace LC::Vrooby
{
	class DevBackend;
	class TrayView;

	class Plugin : public QObject
				, public IInfo
				, public IRemovableDevManager
				, public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IRemovableDevManager IActionsExporter)

		LC_PLUGIN_METADATA ("org.LeechCraft.Vrooby")

		std::shared_ptr<DevBackend> Backend_;
		std::shared_ptr<QAction> ActionDevices_;
		TrayView *TrayView_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		bool SupportsDevType (DeviceType) const override;
		QAbstractItemModel* GetDevicesModel () const override;
		void MountDevice (const QString&) override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;
	private slots:
		void checkAction ();
		void showTrayView ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
