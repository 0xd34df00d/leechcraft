/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <QItemSelection>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/structures.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "dbupdatethreadfwd.h"
#include "feed.h"

namespace LC
{
namespace Util
{
	class ShortcutManager;
}

namespace Aggregator
{
	class AggregatorTab;
	class RepresentationManager;
	class PluginManager;
	class UpdatesManager;
	class ChannelsModel;
	class ResourcesFetcher;
	class OpmlAdder;
	class FeedsErrorManager;
	struct AppWideActions;
	struct ChannelActions;
	struct ItemsWidgetDependencies;

	class Aggregator : public QObject
					 , public IInfo
					 , public IHaveTabs
					 , public IHaveSettings
					 , public IJobHolder
					 , public IEntityHandler
					 , public IHaveShortcuts
					 , public IActionsExporter
					 , public IStartupWizard
					 , public IPluginReady
					 , public IHaveRecoverableTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IHaveSettings
				IJobHolder
				IEntityHandler
				IHaveShortcuts
				IStartupWizard
				IActionsExporter
				IPluginReady
				IHaveRecoverableTabs)

		LC_PLUGIN_METADATA ("org.LeechCraft.Aggregator")

		ICoreProxy_ptr Proxy_;
		std::shared_ptr<AppWideActions> AppWideActions_;
		std::shared_ptr<ChannelActions> ChannelActions_;

		QMenu *ToolMenu_;

		TabClassInfo TabInfo_;

		std::shared_ptr<Util::XmlSettingsDialog> XmlSettingsDialog_;
		std::shared_ptr<RepresentationManager> ReprManager_;
		std::shared_ptr<AggregatorTab> AggregatorTab_;

		Util::ShortcutManager *ShortcutMgr_ = nullptr;

		std::shared_ptr<FeedsErrorManager> ErrorsManager_;
		std::shared_ptr<UpdatesManager> UpdatesManager_;
		std::shared_ptr<PluginManager> PluginManager_;
		std::shared_ptr<ResourcesFetcher> ResourcesFetcher_;

		std::shared_ptr<ChannelsModel> ChannelsModel_;

		std::shared_ptr<OpmlAdder> OpmlAdder_;

		std::shared_ptr<DBUpdateThread> DBUpThread_;

		bool InitFailed_ = false;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QStringList Provides () const override;
		QStringList Needs () const override;
		QStringList Uses () const override;
		QIcon GetIcon () const override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		QAbstractItemModel* GetRepresentation () const override;
		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		EntityTestHandleResult CouldHandle (const Entity&) const override;
		void Handle (Entity) override;

		void SetShortcut (const QString&, const QKeySequences_t&) override;
		QMap<QString, ActionInfo> GetActionInfo () const override;

		QList<QWizardPage*> GetWizardPages () const override;

		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject*) override;

		void RecoverTabs (const QList<TabRecoverInfo>& infos) override;
		bool HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const override;
	private:
		QModelIndex GetRelevantIndex () const;
		QList<QModelIndex> GetRelevantIndexes () const;

		void AddFeed (QString, const QStringList&, const std::optional<Feed::FeedSettings>& = {}) const;

		template<typename F>
		void Perform (F&&);

		void ReinitStorage ();

		ItemsWidgetDependencies MakeItemsWidgetDeps () const;
	private slots:
		void on_ActionMarkAllAsRead__triggered ();
		void on_ActionAddFeed__triggered ();
		void on_ActionRemoveFeed__triggered ();
		void on_ActionRenameFeed__triggered ();
		void on_ActionRemoveChannel__triggered ();
		void on_ActionUpdateSelectedFeed__triggered ();
		void on_ActionImportOPML__triggered ();
		void on_ActionExportOPML__triggered ();
		void on_ActionImportBinary__triggered ();
		void on_ActionExportBinary__triggered ();
		void on_ActionExportFB2__triggered ();
		void on_ActionMarkChannelAsRead__triggered ();
		void on_ActionMarkChannelAsUnread__triggered ();
		void on_ActionChannelSettings__triggered ();
	signals:
		void addNewTab (const QString&, QWidget*) override;
		void removeTab (QWidget*) override;
		void changeTabName (QWidget*, const QString&) override;
		void changeTabIcon (QWidget*, const QIcon&) override;
		void statusBarChanged (QWidget*, const QString&) override;
		void raiseTab (QWidget*) override;

		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
}
