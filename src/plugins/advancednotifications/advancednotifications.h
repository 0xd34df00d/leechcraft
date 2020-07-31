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
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/ipluginready.h>
#include <interfaces/an/ianrulesstorage.h>

namespace LC
{
namespace AdvancedNotifications
{
	class GeneralHandler;
	class RulesManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
				 , public IHaveSettings
				 , public IActionsExporter
				 , public IQuarkComponentProvider
				 , public IPluginReady
				 , public IANRulesStorage
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IEntityHandler
				IHaveSettings
				IActionsExporter
				IQuarkComponentProvider
				IPluginReady
				IANRulesStorage)

		LC_PLUGIN_METADATA ("org.LeechCraft.AdvancedNotifications")

		ICoreProxy_ptr Proxy_;

		RulesManager *RulesManager_;

		Util::XmlSettingsDialog_ptr SettingsDialog_;

		std::shared_ptr<GeneralHandler> GeneralHandler_;

		QuarkComponent_ptr Component_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		QuarkComponents_t GetComponents () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		QList<Entity> GetAllRules (const QString&) const;
		void RequestRuleConfiguration (const Entity&);
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);

		void rulesChanged ();
	};
}
}
