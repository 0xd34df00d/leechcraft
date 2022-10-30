/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QQueue>
#include <QTimer>
#include <QTranslator>
#include <QCursor>
#include <QKeyEvent>
#include <QInputDialog>
#include <QXmlStreamReader>
#include <interfaces/ijobholderrepresentationhandler.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/util.h>
#include <util/db/backendselector.h>
#include <util/gui/util.h>
#include <util/models/flattofoldersproxymodel.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/xpc/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "channelsfiltermodel.h"
#include "aggregator.h"
#include "addfeeddialog.h"
#include "xmlsettingsmanager.h"
#include "feedsettings.h"
#include "export2fb2dialog.h"
#include "channelsmodel.h"
#include "aggregatortab.h"
#include "storagebackendmanager.h"
#include "exportutils.h"
#include "actionsstructs.h"
#include "representationmanager.h"
#include "dbupdatethread.h"
#include "dbupdatethreadworker.h"
#include "pluginmanager.h"
#include "startupfirstpage.h"
#include "startupsecondpage.h"
#include "startupthirdpage.h"
#include "updatesmanager.h"
#include "resourcesfetcher.h"
#include "poolsmanager.h"
#include "opmladder.h"
#include "feedserrormanager.h"

namespace LC
{
namespace Aggregator
{
	void Aggregator::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("aggregator");

		qRegisterMetaType<IDType_t> ("IDType_t");
		qRegisterMetaType<QList<IDType_t>> ("QList<IDType_t>");
		qRegisterMetaType<QSet<IDType_t>> ("QSet<IDType_t>");
		qRegisterMetaType<QItemSelection> ("QItemSelection");
		qRegisterMetaType<Item> ("Item");
		qRegisterMetaType<ChannelShort> ("ChannelShort");
		qRegisterMetaType<Channel> ("Channel");
		qRegisterMetaType<channels_container_t> ("channels_container_t");

		TabInfo_ = TabClassInfo
		{
			"Aggregator",
			GetName (),
			GetInfo (),
			GetIcon (),
			0,
			TFSingle | TFOpenableByRequest
		};

		ShortcutMgr_ = new Util::ShortcutManager (GetProxyHolder (), this);

		ChannelActions_ = std::make_shared<ChannelActions> (ShortcutMgr_, this);
		AppWideActions_ = std::make_shared<AppWideActions> (ShortcutMgr_, this);

		ToolMenu_ = AppWideActions_->CreateToolMenu ();
		ToolMenu_->setIcon (GetIcon ());

		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), "aggregatorsettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
				new Util::BackendSelector (XmlSettingsManager::Instance ()));

		ReinitStorage ();

		OpmlAdder_ = std::make_shared<OpmlAdder> (std::bind_front (&Aggregator::AddFeed, this));

		DBUpThread_ = std::make_shared<DBUpdateThread> ();
		DBUpThread_->SetAutoQuit (true);
		DBUpThread_->start (QThread::LowestPriority);

		PoolsManager::Instance ().ReloadPools ();

		ErrorsManager_ = std::make_shared<FeedsErrorManager> ();

		UpdatesManager_ = std::make_shared<UpdatesManager> (UpdatesManager::InitParams {
					DBUpThread_,
					ErrorsManager_,
					GetProxyHolder ()->GetEntityManager ()
				});

		connect (AppWideActions_->ActionUpdateFeeds_,
				&QAction::triggered,
				UpdatesManager_.get (),
				&UpdatesManager::UpdateFeeds);

		QMetaObject::connectSlotsByName (this);

		ChannelsModel_ = std::make_shared<ChannelsModel> (ErrorsManager_, GetProxyHolder ()->GetTagsManager ());

		PluginManager_ = std::make_shared<PluginManager> (ChannelsModel_.get ());
		PluginManager_->RegisterHookable (&StorageBackendManager::Instance ());

		ResourcesFetcher_ = std::make_shared<ResourcesFetcher> (GetProxyHolder ()->GetEntityManager ());
	}

	void Aggregator::SecondInit ()
	{
		ReprManager_ = std::make_shared<RepresentationManager> (RepresentationManager::InitParams {
					*AppWideActions_,
					*ChannelActions_,
					ChannelsModel_.get (),
					MakeItemsWidgetDeps ()
				});
	}

	void Aggregator::Release ()
	{
		PluginManager_.reset ();
		ReprManager_.reset ();
		AggregatorTab_.reset ();
		ChannelsModel_.reset ();
		DBUpThread_.reset ();
		StorageBackendManager::Instance ().Release ();
	}

	QByteArray Aggregator::GetUniqueID () const
	{
		return "org.LeechCraft.Aggregator";
	}

	QString Aggregator::GetName () const
	{
		return "Aggregator";
	}

	QString Aggregator::GetInfo () const
	{
		return tr ("RSS/Atom feed reader.");
	}

	QStringList Aggregator::Provides () const
	{
		return { "rss" };
	}

	QStringList Aggregator::Needs () const
	{
		return { "http" };
	}

	QStringList Aggregator::Uses () const
	{
		return { "webbrowser" };
	}

	QIcon Aggregator::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Aggregator::GetTabClasses () const
	{
		return { TabInfo_ };
	}

	void Aggregator::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass != "Aggregator")
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;

		if (!AggregatorTab_)
			AggregatorTab_ = std::make_unique<AggregatorTab> (AggregatorTab::InitParams {
						*AppWideActions_,
						ChannelActions_,
						TabInfo_,
						ChannelsModel_.get (),
						Proxy_->GetTagsManager (),
						MakeItemsWidgetDeps ()
					},
					this);

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (AggregatorTab_->GetTabClassInfo ().VisibleName_,
				AggregatorTab_.get ());
	}

	Util::XmlSettingsDialog_ptr Aggregator::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QAbstractItemModel* Aggregator::GetRepresentation () const
	{
		return ReprManager_->GetRepresentation ();
	}

	IJobHolderRepresentationHandler_ptr Aggregator::CreateRepresentationHandler ()
	{
		// TODO create per-call
		return ReprManager_;
	}

	EntityTestHandleResult Aggregator::CouldHandle (const Entity& e) const
	{
		if (!e.Entity_.canConvert<QUrl> ())
			return {};

		if (OpmlAdder_->IsOpmlEntity (e))
			return EntityTestHandleResult { EntityTestHandleResult::PIdeal };

		const auto& url = e.Entity_.toUrl ();
		if (e.Mime_ == "text/xml")
		{
			if (url.scheme () != "http" &&
					url.scheme () != "https")
				return {};

			const auto& pageData = e.Additional_ ["URLData"].toString ();
			QXmlStreamReader xmlReader (pageData);
			if (!xmlReader.readNextStartElement ())
				return {};

			return xmlReader.name () == "rss" || xmlReader.name () == "atom" ?
					EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
					EntityTestHandleResult {};
		}
		else
		{
			if (url.scheme () == "feed" || url.scheme () == "itpc")
				return EntityTestHandleResult { EntityTestHandleResult::PIdeal };

			if (url.scheme () != "http" &&
					url.scheme () != "https")
				return {};

			if (e.Mime_ != "application/atom+xml" &&
					e.Mime_ != "application/rss+xml")
				return {};

			const auto& linkRel = e.Additional_ ["LinkRel"].toString ();
			if (!linkRel.isEmpty () &&
					linkRel != "alternate")
				return {};
		}

		return EntityTestHandleResult { EntityTestHandleResult::PIdeal };
	}

	void Aggregator::Handle (Entity e)
	{
		if (OpmlAdder_->HandleOpmlEntity (e))
			return;

		QUrl url = e.Entity_.toUrl ();
		QString str = url.toString ();
		if (str.startsWith ("feed://"))
			str.replace (0, 4, "http");
		else if (str.startsWith ("feed:"))
			str.remove  (0, 5);
		else if (str.startsWith ("itpc://"))
			str.replace (0, 4, "http");

		AddFeedDialog af { Proxy_->GetTagsManager (), str };
		if (af.exec () == QDialog::Accepted)
			AddFeed (af.GetURL (), af.GetTags ());
	}

	void Aggregator::SetShortcut (const QString& name, const QKeySequences_t& shortcuts)
	{
		ShortcutMgr_->SetShortcut (name, shortcuts);
	}

	QMap<QString, ActionInfo> Aggregator::GetActionInfo () const
	{
		return ShortcutMgr_->GetActionInfo ();
	}

	QList<QWizardPage*> Aggregator::GetWizardPages () const
	{
		QList<QWizardPage*> result;
		int version = XmlSettingsManager::Instance ()->Property ("StartupVersion", 0).toInt ();
		if (version <= 0)
			result << new StartupFirstPage ();
		if (version <= 1)
			result << new StartupSecondPage ();
		if (version <= 2)
		{
			auto third = new StartupThirdPage ();
			result << third;

			connect (third,
					&StartupThirdPage::feedsSelected,
					this,
					[this] (const QList<StartupThirdPage::SelectedFeed>& feeds)
					{
						auto tm = Proxy_->GetTagsManager ();
						for (const auto& feed : feeds)
							AddFeed (feed.URL_, tm->Split (feed.Tags_));
					});

			connect (third,
					&StartupThirdPage::reinitStorageRequested,
					this,
					&Aggregator::ReinitStorage);
		}
		return result;
	}

	QList<QAction*> Aggregator::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		switch (place)
		{
		case ActionsEmbedPlace::ToolsMenu:
			result << ToolMenu_->menuAction ();
			break;
		case ActionsEmbedPlace::CommonContextMenu:
			result << AppWideActions_->ActionAddFeed_;
			result << AppWideActions_->ActionUpdateFeeds_;
			break;
		default:
			break;
		}

		return result;
	}

	QSet<QByteArray> Aggregator::GetExpectedPluginClasses () const
	{
		return { "org.LeechCraft.Aggregator.GeneralPlugin/1.0" };
	}

	void Aggregator::AddPlugin (QObject *plugin)
	{
		PluginManager_->AddPlugin (plugin);
	}

	void Aggregator::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		for (const auto& recInfo : infos)
		{
			if (recInfo.Data_ == "aggregatortab")
			{
				for (const auto& pair : recInfo.DynProperties_)
					setProperty (pair.first, pair.second);

				TabOpenRequested (TabInfo_.TabClass_);
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown context"
						<< recInfo.Data_;
		}
	}

	bool Aggregator::HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const
	{
		return true;
	}

	QModelIndex Aggregator::GetRelevantIndex () const
	{
		if (const auto idx = ReprManager_->GetRelevantIndex ())
			return *idx;
		else
			return AggregatorTab_->GetRelevantIndex ();
	}

	QList<QModelIndex> Aggregator::GetRelevantIndexes () const
	{
		if (const auto idx = ReprManager_->GetRelevantIndex ())
			return { *idx };
		else
			return AggregatorTab_->GetRelevantIndexes ();
	}

	void Aggregator::AddFeed (QString url, const QStringList& tags, const std::optional<Feed::FeedSettings>& maybeFeedSettings) const
	{
		auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const auto& fixedUrl = QUrl::fromUserInput (url);
		url = fixedUrl.toString ();
		if (sb->FindFeed (url))
		{
			auto e = Util::MakeNotification (tr ("Feed addition error"),
					tr ("The feed %1 is already added")
							.arg (url),
					Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			return;
		}

		Feed feed;
		feed.URL_ = url;
		sb->AddFeed (feed);
		sb->SetFeedTags (feed.FeedID_, Proxy_->GetTagsManager ()->GetIDs (tags));

		if (maybeFeedSettings)
		{
			auto fs = *maybeFeedSettings;
			fs.FeedID_ = feed.FeedID_;
			sb->SetFeedSettings (fs);
		}

		UpdatesManager_->UpdateFeed (feed.FeedID_);
	}

	void Aggregator::on_ActionMarkAllAsRead__triggered ()
	{
		if (XmlSettingsManager::Instance ()->property ("ConfirmMarkAllAsRead").toBool ())
		{
			QMessageBox mbox (QMessageBox::Question,
					"LeechCraft",
					tr ("Do you really want to mark all channels as read?"),
					QMessageBox::Yes | QMessageBox::No,
					nullptr);
			mbox.setDefaultButton (QMessageBox::No);

			QPushButton always (tr ("Always"));
			mbox.addButton (&always, QMessageBox::AcceptRole);

			if (mbox.exec () == QMessageBox::No)
				return;
			else if (mbox.clickedButton () == &always)
				XmlSettingsManager::Instance ()->setProperty ("ConfirmMarkAllAsRead", false);
		}

		for (int i = 0; i < ChannelsModel_->rowCount (); ++i)
			DBUpThread_->ToggleChannelUnread (ChannelsModel_->index (i, 0), false);
	}

	void Aggregator::on_ActionAddFeed__triggered ()
	{
		AddFeedDialog af { Proxy_->GetTagsManager () };
		if (af.exec () == QDialog::Accepted)
			AddFeed (af.GetURL (), af.GetTags ());
	}

	void Aggregator::on_ActionRemoveFeed__triggered ()
	{
		const auto& ds = GetRelevantIndex ();
		if (!ds.isValid ())
			return;

		const auto& name = ds.sibling (ds.row (), ChannelsModel::ColumnTitle).data ().toString ();
		if (QMessageBox::question (nullptr,
					tr ("Feed deletion"),
					tr ("Are you sure you want to delete feed %1?")
						.arg (Util::FormatName (name)),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;

		const auto feedId = ds.data (ChannelRoles::FeedID).value<IDType_t> ();
		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->RemoveFeed (feedId);
	}

	void Aggregator::on_ActionRenameFeed__triggered ()
	{
		const auto& ds = GetRelevantIndex ();
		if (!ds.isValid ())
			return;

		const auto& current = ds.sibling (ds.row (), ChannelsModel::ColumnTitle).data ().toString ();
		const auto& newName = QInputDialog::getText (nullptr,
				tr ("Rename feed"),
				tr ("New feed name:"),
				QLineEdit::Normal,
				current);
		if (newName.isEmpty ())
			return;

		auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetChannelDisplayTitle (ds.data (ChannelRoles::ChannelID).value<IDType_t> (), newName);
	}

	void Aggregator::on_ActionRemoveChannel__triggered ()
	{
		const auto& ds = GetRelevantIndex ();
		if (!ds.isValid ())
			return;

		const auto& name = ds.sibling (ds.row (), ChannelsModel::ColumnTitle).data ().toString ();
		if (QMessageBox::question (nullptr,
				tr ("Channel deletion"),
				tr ("Are you sure you want to delete channel %1?")
					.arg (Util::FormatName (name)),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;

		const auto channelId = ds.data (ChannelRoles::ChannelID).value<IDType_t> ();
		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->RemoveChannel (channelId);
	}

	template<typename F>
	void Aggregator::Perform (F&& func)
	{
		for (auto index : GetRelevantIndexes ())
			func (index);
	}

	void Aggregator::ReinitStorage ()
	{
		const auto storageReady = Util::Visit (StorageBackendManager::Instance ().CreatePrimaryStorage (),
				[] (const StorageBackend_ptr&) { return true; },
				[] (const auto& error)
				{
					auto box = new QMessageBox (QMessageBox::Critical,
							"LeechCraft",
							tr ("Failed to initialize Aggregator storage: %1.")
								.arg (error.Message_),
							QMessageBox::Ok);
					box->open ();
					return false;
				});

		AppWideActions_->SetEnabled (storageReady);
	}

	ItemsWidgetDependencies Aggregator::MakeItemsWidgetDeps () const
	{
		return
		{
			ShortcutMgr_,
			ChannelsModel_.get (),
			*AppWideActions_,
			*ChannelActions_,
			[this] (const QString& url, const QStringList& tags) { AddFeed (url, tags); }
		};
	}

	namespace
	{
		QString FormatNamesList (const QStringList& names)
		{
			return "<em>" + names.join ("</em>; <em>") + "</em>";
		}
	}

	void Aggregator::on_ActionMarkChannelAsRead__triggered ()
	{
		QStringList names;
		Perform ([&names] (const QModelIndex& mi)
				{ names << mi.sibling (mi.row (), 0).data ().toString (); });
		if (XmlSettingsManager::Instance ()->Property ("ConfirmMarkChannelAsRead", true).toBool ())
		{
			QMessageBox mbox (QMessageBox::Question,
					"LeechCraft",
					tr ("Are you sure you want to mark all items in %1 as read?")
						.arg (FormatNamesList (names)),
					QMessageBox::Yes | QMessageBox::No);

			mbox.setDefaultButton (QMessageBox::Yes);

			QPushButton always (tr ("Always"));
			mbox.addButton (&always, QMessageBox::AcceptRole);

			if (mbox.exec () == QMessageBox::No)
				return;
			else if (mbox.clickedButton () == &always)
				XmlSettingsManager::Instance ()->setProperty ("ConfirmMarkChannelAsRead", false);
		}

		Perform ([this] (const QModelIndex& mi) { DBUpThread_->ToggleChannelUnread (mi, false); });
	}

	void Aggregator::on_ActionMarkChannelAsUnread__triggered ()
	{
		QStringList names;
		Perform ([&names] (const QModelIndex& mi)
				{ names << mi.sibling (mi.row (), 0).data ().toString (); });
		if (QMessageBox::question (nullptr,
				"LeechCraft",
				tr ("Are you sure you want to mark all items in %1 as unread?")
					.arg (FormatNamesList (names)),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Perform ([this] (const QModelIndex& mi) { DBUpThread_->ToggleChannelUnread (mi, true); });
	}

	void Aggregator::on_ActionChannelSettings__triggered ()
	{
		QModelIndex index = GetRelevantIndex ();
		if (!index.isValid ())
			return;

		FeedSettings dia { index, Proxy_ };
		connect (&dia,
				&FeedSettings::faviconRequested,
				ResourcesFetcher_.get (),
				&ResourcesFetcher::FetchFavicon);
		dia.exec ();
	}

	void Aggregator::on_ActionUpdateSelectedFeed__triggered ()
	{
		Perform ([this] (const QModelIndex& mi)
				{
					UpdatesManager_->UpdateFeed (mi.data (ChannelRoles::FeedID).value<IDType_t> ());
				});
	}

	void Aggregator::on_ActionImportOPML__triggered ()
	{
		OpmlAdder_->StartAddingOpml ({});
	}

	void Aggregator::on_ActionExportOPML__triggered ()
	{
		ExportUtils::RunExportOPML ();
	}

	void Aggregator::on_ActionExportFB2__triggered ()
	{
		const auto dialog = new Export2FB2Dialog (ChannelsModel_.get (), nullptr);
		dialog->setAttribute (Qt::WA_DeleteOnClose);
		dialog->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_aggregator, LC::Aggregator::Aggregator);
