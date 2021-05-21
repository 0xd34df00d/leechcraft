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

namespace LC::AdvancedNotifications
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

		RulesManager *RulesManager_;

		Util::XmlSettingsDialog_ptr SettingsDialog_;

		std::shared_ptr<GeneralHandler> GeneralHandler_;

		QuarkComponent_ptr Component_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		QuarkComponents_t GetComponents () const override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject*) override;

		QList<Entity> GetAllRules (const QString&) const override;
		void RequestRuleConfiguration (const Entity&) override;
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;

		void rulesChanged () override;
	};
}
