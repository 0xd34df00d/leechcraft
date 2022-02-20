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
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ijobholder.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ihavediaginfo.h>

namespace LC
{
namespace LMP
{
	class PlayerTab;
	class EffectsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IHaveSettings
				 , public IEntityHandler
				 , public IActionsExporter
				 , public IHaveRecoverableTabs
				 , public IHaveShortcuts
				 , public IPluginReady
				 , public IJobHolder
				 , public IDataFilter
				 , public IHaveDiagInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IHaveSettings
				IEntityHandler
				IActionsExporter
				IHaveRecoverableTabs
				IHaveShortcuts
				IPluginReady
				IJobHolder
				IDataFilter
				IHaveDiagInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP")

		ICoreProxy_ptr Proxy_;

		TabClassInfo PlayerTC_;

		PlayerTab *PlayerTab_;

		Util::XmlSettingsDialog_ptr XSD_;

		QAction *ActionRescan_;
		QAction *ActionCollectionStats_;

		QMap<QString, Entity> GlobAction2Entity_;
		QMap<QString, ActionInfo> GlobAction2Info_;

		EffectsManager *EffectsMgr_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		QList<QAction*> GetActions (ActionsEmbedPlace area) const override;
		QMap<QString, QList<QAction*>> GetMenuActions () const override;

		void RecoverTabs (const QList<TabRecoverInfo>& infos) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;

		void SetShortcut (const QString&, const QKeySequences_t&) override;
		QMap<QString, ActionInfo> GetActionInfo () const override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject* plugin) override;

		QAbstractItemModel* GetRepresentation () const override;

		QString GetFilterVerb () const override;
		QList<FilterVariant> GetFilterVariants (const QVariant&) const override;

		QString GetDiagInfoString () const override;
	private:
		void InitShortcuts ();
	private slots:
		void handleFullRaiseRequested ();
		void showCollectionStats ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
}
