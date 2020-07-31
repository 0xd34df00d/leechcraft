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
#include "batteryhistory.h"
#include "batteryinfo.h"

namespace LC
{
namespace Liznoo
{
	class BatteryHistoryDialog;
	class PlatformObjects;

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

		ICoreProxy_ptr Proxy_;

		Util::XmlSettingsDialog_ptr XSD_;

		std::shared_ptr<PlatformObjects> Platform_;

		QMap<QString, BatteryInfo> Battery2LastInfo_;
		QMap<QString, BatteryHistoryDialog*> Battery2Dialog_;
		QMap<QString, BatteryHistoryList> Battery2History_;

		QAction *Suspend_;
		QAction *Hibernate_;

		QuarkComponent_ptr LiznooQuark_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		EntityTestHandleResult CouldHandle (const Entity& entity) const;
		void Handle (Entity entity);

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
		QMap<QString, QList<QAction*>> GetMenuActions () const;

		QuarkComponents_t GetComponents () const;
	private:
		void CheckNotifications (const BatteryInfo&);
	private slots:
		void handleBatteryInfo (Liznoo::BatteryInfo);
		void handleUpdateHistory ();
		void handleHistoryTriggered ();
		void handleHistoryTriggered (const QString&);

		void handleSuspendRequested ();
		void handleHibernateRequested ();

		void handlePushButton (const QString&);
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}

