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
#include <QCompleter>
#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QQueue>
#include <QTimer>
#include <QTranslator>
#include <QCursor>
#include <QKeyEvent>
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
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "components/actions/appwideactions.h"
#include "components/actions/channelactions.h"
#include "channelsfiltermodel.h"
#include "aggregator.h"
#include "addfeeddialog.h"
#include "xmlsettingsmanager.h"
#include "feedsettings.h"
#include "channelsmodel.h"
#include "aggregatortab.h"
#include "storagebackendmanager.h"
#include "representationmanager.h"
#include "dbupdatethread.h"
#include "pluginmanager.h"
#include "startupfirstpage.h"
#include "startupsecondpage.h"
#include "startupthirdpage.h"
#include "updatesmanager.h"
#include "resourcesfetcher.h"
#include "poolsmanager.h"
#include "opmladder.h"
#include "feedserrormanager.h"
#include "dbutils.h"

namespace LC
{
namespace Aggregator
{
	void Aggregator::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("aggregator");

		qRegisterMetaType<IDType_t> ("IDType_t");
		qRegisterMetaType<QList<IDType_t>> ("QList<IDType_t>");
		qRegisterMetaType<QSet<IDType_t>> ("QSet<IDType_t>");
		qRegisterMetaType<Item> ("Item");
		qRegisterMetaType<ChannelShort> ("ChannelShort");
		qRegisterMetaType<Channel> ("Channel");
		qRegisterMetaType<channels_container_t> ("channels_container_t");
		qRegisterMetaType<UnreadChange> ("UnreadChange");

		TabInfo_ = TabClassInfo
		{
			"Aggregator",
			GetName (),
			GetInfo (),
			GetIcon (),
			0,
			TFSingle | TFOpenableByRequest
		};

		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), "aggregatorsettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
				new Util::BackendSelector (XmlSettingsManager::Instance ()));

		DBUpThread_ = std::make_shared<DBUpdateThread> ();

		ErrorsManager_ = std::make_shared<FeedsErrorManager> ();

		UpdatesManager_ = std::make_shared<UpdatesManager> (UpdatesManager::InitParams {
					DBUpThread_,
					ErrorsManager_,
					GetProxyHolder ()->GetEntityManager ()
				});

		ShortcutMgr_ = new Util::ShortcutManager (GetProxyHolder (), this);
		ChannelActions::RegisterActions (*ShortcutMgr_);

		ChannelsModel_ = std::make_shared<ChannelsModel> (ErrorsManager_, GetProxyHolder ()->GetTagsManager ());
		ResourcesFetcher_ = std::make_shared<ResourcesFetcher> (GetProxyHolder ()->GetEntityManager ());

		AppWideActions_ = std::make_shared<AppWideActions> (AppWideActions::Deps {
					.ShortcutManager_ = *ShortcutMgr_,
					.UpdatesManager_ = *UpdatesManager_,
					.DBUpThread_ = *DBUpThread_,
					.ChannelsModel_ = *ChannelsModel_,
				});

		ReinitStorage ();

		PoolsManager::Instance ().ReloadPools ();

		PluginManager_ = std::make_shared<PluginManager> (ChannelsModel_.get ());
		PluginManager_->RegisterHookable (&StorageBackendManager::Instance ());
	}

	void Aggregator::SecondInit ()
	{
		ReprManager_ = std::make_shared<RepresentationManager> (RepresentationManager::Deps {
					.ShortcutManager_ = *ShortcutMgr_,
					.AppWideActions_ = *AppWideActions_,
					.ChannelsModel_ = *ChannelsModel_,
					.UpdatesManager_ = *UpdatesManager_,
					.ResourcesFetcher_ = *ResourcesFetcher_,
					.DBUpThread_ = *DBUpThread_,
				});
	}

	void Aggregator::Release ()
	{
		PluginManager_.reset ();
		ReprManager_.reset ();
		AggregatorTab_.reset ();
		ChannelsModel_.reset ();
		DBUpThread_.reset ();
		AppWideActions_.reset ();
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
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
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
						.TabClass_ = TabInfo_,
						.AppWideActions_ = *AppWideActions_,
						.ChannelsModel_ = *ChannelsModel_,
						.ShortcutManager_ = *ShortcutMgr_,
						.UpdatesManager_ = *UpdatesManager_,
						.ResourcesFetcher_ = *ResourcesFetcher_,
						.DBUpThread_ = *DBUpThread_,
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

		if (Opml::IsOpmlEntity (e))
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
		if (Opml::IsOpmlEntity (e))
		{
			Opml::HandleOpmlEntity (e, UpdatesManager_);
			return;
		}

		QUrl url = e.Entity_.toUrl ();
		QString str = url.toString ();
		if (str.startsWith ("feed://"))
			str.replace (0, 4, "http");
		else if (str.startsWith ("feed:"))
			str.remove  (0, 5);
		else if (str.startsWith ("itpc://"))
			str.replace (0, 4, "http");

		AddFeedDialog af { str };
		if (af.exec () == QDialog::Accepted)
			AddFeed ({ .URL_ = af.GetURL (), .Tags_ = af.GetTags (), .UpdatesManager_ = *UpdatesManager_ });
	}

	void Aggregator::SetShortcut (const QByteArray& name, const QKeySequences_t& shortcuts)
	{
		ShortcutMgr_->SetShortcut (name, shortcuts);
	}

	QMap<QByteArray, ActionInfo> Aggregator::GetActionInfo () const
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
						auto tm = GetProxyHolder ()->GetTagsManager ();
						for (const auto& feed : feeds)
							AddFeed ({
										.URL_ = feed.URL_,
										.Tags_ = tm->Split (feed.Tags_),
										.UpdatesManager_ = *UpdatesManager_,
									});
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
		return AppWideActions_->GetActions (place);
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

	void Aggregator::ReinitStorage ()
	{
		const auto storageReady = Util::Visit (StorageBackendManager::Instance ().CreatePrimaryStorage (),
				[] (const StorageBackend_ptr&) { return true; },
				[] (const auto& error)
				{
					auto box = new QMessageBox (QMessageBox::Critical,
							MessageBoxTitle,
							tr ("Failed to initialize Aggregator storage: %1.")
								.arg (error.Message_),
							QMessageBox::Ok);
					box->open ();
					return false;
				});

		AppWideActions_->SetEnabled (storageReady);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_aggregator, LC::Aggregator::Aggregator);
