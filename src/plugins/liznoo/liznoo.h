/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <util/threads/coro/sharedtask.h>
#include "platform/poweractions/platform.h"
#include "batteryhistory.h"
#include "batteryinfo.h"

namespace LC
{
namespace Liznoo
{
	class BatteryHistoryDialog;
	class PlatformObjects;
	class QuarkManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IEntityHandler
				 , public IActionsExporter
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IEntityHandler IActionsExporter IQuarkComponentProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Liznoo")

		Util::XmlSettingsDialog_ptr XSD_;

		std::optional<Util::SharedContextTask<void>> PlatformInitTask_;
		std::unique_ptr<PlatformObjects> Platform_;

		QMap<QString, BatteryInfo> Battery2LastInfo_;
		QMap<QString, BatteryHistoryDialog*> Battery2Dialog_;
		QMap<QString, BatteryHistoryList> Battery2History_;

		QAction *Suspend_;
		QAction *Hibernate_;

		QuarkComponent_ptr LiznooQuark_;
	public:
		Plugin ();
		~Plugin () override;

		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		EntityTestHandleResult CouldHandle (const Entity& entity) const override;
		void Handle (Entity entity) override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;
		QMap<QString, QList<QAction*>> GetMenuActions () const override;

		QuarkComponents_t GetComponents () const override;
	private:
		void CheckNotifications (const BatteryInfo&);
		Util::Task<void> EnsurePlatformReady ();

		Util::SharedContextTask<void> InitializePlatform (QPointer<QuarkManager>);
		Util::ContextTask<void> HandleStateRequested (PowerActions::Platform::State);
		Util::ContextTask<void> HandleSettingsButton (QString);
	private slots:
		void handleUpdateHistory ();
		void handleHistoryTriggered ();
		void handleHistoryTriggered (const QString&);
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
}
