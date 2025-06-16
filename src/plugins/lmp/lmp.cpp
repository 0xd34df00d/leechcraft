/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lmp.h"
#include <QIcon>
#include <QFileInfo>
#include <QUrl>
#include <QGraphicsEffect>
#include <QMessageBox>
#include <QAction>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/entityconstants.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include <util/sll/slotclosure.h>
#include <util/sys/util.h>
#include "gstfix.h"
#include "playertab.h"
#include "player.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "rootpathsettingsmanager.h"
#include "collectionstatsdialog.h"
#include "artistbrowsertab.h"
#include "progressmanager.h"
#include "volumenotifycontroller.h"
#include "radiomanager.h"
#include "notificationplayer.h"
#include "effectsmanager.h"
#include "lmpproxy.h"
#include "diaginfocollector.h"

#ifdef ENABLE_MPRIS
#include "mpris/instance.h"
#endif

typedef QList<QPair<QString, QUrl>> CustomStationsList_t;
Q_DECLARE_METATYPE (CustomStationsList_t);

namespace LC
{
namespace LMP
{
	namespace
	{
		void FixGstPaths ()
		{
#if defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
			if (!Util::IsOSXLoadFromBundle ())
				return;

			auto updateEnv = [] (const char *name, const QByteArray& relpath)
			{
				if (qgetenv (name).isEmpty ())
					qputenv (name, QCoreApplication::applicationDirPath ().toUtf8 () + relpath);
			};

			updateEnv ("GST_PLUGIN_SYSTEM_PATH", "/../PlugIns/gstreamer");
			updateEnv ("GST_PLUGIN_SCANNER", "gst-plugin-scanner");
			updateEnv ("GTK_PATH", "/../Frameworks");
			updateEnv ("GIO_EXTRA_MODULES", "/../PlugIns/gstreamer");
			updateEnv ("GSETTINGS_SCHEMA_DIR", "/../Frameworks/schemas");

			qputenv ("GST_REGISTRY_FORK", "no");
#endif
		}
	}

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		FixGstPaths ();

		gint argc = 1;
		gchar *argvarr [] = { g_strdup ("leechcraft"), nullptr };
		gchar **argv = argvarr;
		gst_init (&argc, &argv);

		qRegisterMetaType<QList<QPair<QString, QUrl>>> ("QList<QPair<QString, QUrl>>");
		qRegisterMetaType<SavedFilterInfo> ("LC::LMP::SavedFilterInfo");
		qRegisterMetaType<QList<SavedFilterInfo>> ("QList<LC::LMP::SavedFilterInfo>");
#if QT_VERSION_MAJOR < 6
		qRegisterMetaTypeStreamOperators<QList<QPair<QString, QUrl>>> ();
		qRegisterMetaTypeStreamOperators<SavedFilterInfo> ();
		qRegisterMetaTypeStreamOperators<QList<SavedFilterInfo>> ();
#endif

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpsettings.xml");

		PlayerTC_ =
		{
			GetUniqueID () + "_player",
			"LMP",
			GetInfo (),
			GetIcon (),
			40,
			TFSingle | TFByDefault | TFOpenableByRequest
		};

		Core::Instance ().InitWithProxy ();

		auto mgr = new RootPathSettingsManager (this);
		XSD_->SetDataSource ("RootPathsView", mgr->GetModel ());

		PlayerTab_ = new PlayerTab (PlayerTC_, Core::Instance ().GetPlayer (), proxy, this);

		Core::Instance ().GetLmpProxy ()->GetGuiProxy ()->SetPlayerTab (PlayerTab_);

		EffectsMgr_ = new EffectsManager (Core::Instance ().GetPlayer ()->GetPath (), this);
		XSD_->SetDataSource ("EffectsView", EffectsMgr_->GetEffectsModel ());
		connect (EffectsMgr_,
				SIGNAL (effectsListChanged (QStringList)),
				PlayerTab_,
				SLOT (updateEffectsList (QStringList)));
		connect (PlayerTab_,
				SIGNAL (effectsConfigRequested (int)),
				EffectsMgr_,
				SLOT (showEffectConfig (int)));

		connect (PlayerTab_,
				SIGNAL (fullRaiseRequested ()),
				this,
				SLOT (handleFullRaiseRequested ()));

		ActionRescan_ = new QAction (tr ("Rebuild collection"), this);
		ActionRescan_->setProperty ("ActionIcon", "view-refresh");
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[]
			{
				if (QMessageBox::question (nullptr,
						"LeechCraft",
						tr ("Are you sure you want to rebuild the collection? "
							"This will reset all the play history and counts.")) == QMessageBox::Yes)
					Core::Instance ().rescan ();
			},
			ActionRescan_,
			SIGNAL (triggered ()),
			ActionRescan_
		};

		ActionCollectionStats_ = new QAction (tr ("Collection statistics"), this);
		ActionCollectionStats_->setProperty ("ActionIcon", "view-statistics");
		connect (ActionCollectionStats_,
				SIGNAL (triggered ()),
				this,
				SLOT (showCollectionStats ()));

		InitShortcuts ();

#ifdef ENABLE_MPRIS
		const auto mpris = new MPRIS::Instance (Core::Instance ().GetPlayer (), this);
		connect (mpris,
				&MPRIS::Instance::raiseRequested,
				this,
				[this] { GetProxyHolder ()->GetRootWindowsManager ()->AddTab ("LMP", PlayerTab_); });
#endif
	}

	void Plugin::SecondInit ()
	{
		for (const auto& e : GlobAction2Entity_)
			Proxy_->GetEntityManager ()->HandleEntity (e);

		Core::Instance ().InitWithOtherPlugins ();
		PlayerTab_->InitWithOtherPlugins ();

		EffectsMgr_->RegisteringFinished ();
	}

	void Plugin::SetShortcut (const QByteArray& id, const QKeySequences_t& sequences)
	{
		if (!GlobAction2Entity_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown id"
					<< id;
			return;
		}

		auto& e = GlobAction2Entity_ [id];
		e.Additional_ [EF::GlobalAction::Shortcut] = QVariant::fromValue (sequences.value (0));
		e.Additional_ [EF::GlobalAction::AltShortcuts] = Util::Map (sequences.mid (1),
				[] (const auto& seq) { return QVariant::fromValue (seq); });
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	QMap<QByteArray, ActionInfo> Plugin::GetActionInfo () const
	{
		return GlobAction2Info_;
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP";
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QString Plugin::GetName () const
	{
		return "LMP";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LeechCraft Music Player.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/lmp/resources/images/lmp.svg");
		return icon;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return
		{
			PlayerTC_,
			ArtistBrowserTab::GetStaticTabClass ()
		};
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == PlayerTC_.TabClass_)
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab ("LMP", PlayerTab_);
		else if (tc == ArtistBrowserTab::GetStaticTabClass ().TabClass_)
			new ArtistBrowserTab {};
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (e.Mime_ == Mimes::PowerStateChanged)
			return EntityTestHandleResult { EntityTestHandleResult::PHigh };

		if (e.Mime_ == Mimes::DataFilterRequest)
		{
			if (!e.Additional_ ["DataFilter"].toString ().startsWith (GetUniqueID ()))
				return {};

			if (e.Entity_.typeId () != QMetaType::QString)
				return {};

			if (e.Entity_.toString ().size () >= 80)
				return {};

			return EntityTestHandleResult { EntityTestHandleResult::PHigh };
		}

		QString path = e.Entity_.toString ();
		const QUrl& url = e.Entity_.toUrl ();
		if (path.isEmpty () &&
				url.isValid () &&
				url.scheme () == "file")
			path = url.toLocalFile ();

		const auto& goodExt = XmlSettingsManager::Instance ()
				.property ("TestExtensions").toString ()
				.split (' ', Qt::SkipEmptyParts);
		const QFileInfo fi (path);
		if ((fi.exists () && goodExt.contains (fi.suffix ())) ||
				e.Additional_ ["Action"] == "AudioEnqueuePlay" ||
				e.Additional_ ["Action"] == "AudioEnqueue")
			return EntityTestHandleResult { EntityTestHandleResult::PHigh };
		else
			return {};
	}

	void Plugin::Handle (Entity e)
	{
		auto player = PlayerTab_->GetPlayer ();

		if (e.Mime_ == Mimes::PowerStateChanged)
		{
			if (e.Entity_ == PowerState::Sleeping)
			{
				player->SavePlayState (true);
				player->setPause ();
			}
			else if (e.Entity_ == PowerState::WokeUp)
			{
				player->RestorePlayState ();
				Core::Instance ().GetRadioManager ()->HandleWokeUp ();
			}

			return;
		}
		if (e.Mime_ == Mimes::DataFilterRequest)
		{
			new ArtistBrowserTab { e.Entity_.toString ().trimmed () };
			return;
		}

		QString path = e.Entity_.toString ();
		const QUrl& url = e.Entity_.toUrl ();
		if (path.isEmpty () &&
				url.isValid () &&
				url.scheme () == "file")
			path = url.toLocalFile ();

		if (e.Parameters_ & Internal)
		{
			new NotificationPlayer (path, Proxy_, this);
			return;
		}

		if (!(e.Parameters_ & FromUserInitiated))
			return;

		player->Enqueue ({ AudioSource (url) }, Player::EnqueueNone);

		if (e.Additional_ ["Action"] == "AudioEnqueuePlay")
			player->AddToOneShotQueue (url);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		return
		{
			{
				GetName (),
				{ ActionRescan_, ActionCollectionStats_ }
			}
		};
	}

	void Plugin::RecoverTabs (const QList<LC::TabRecoverInfo>& infos)
	{
		for (const auto& recInfo : infos)
		{
			QDataStream stream (recInfo.Data_);
			QByteArray key;
			stream >> key;

			if (recInfo.Data_ == "playertab")
			{
				for (const auto& pair : recInfo.DynProperties_)
					PlayerTab_->setProperty (pair.first, pair.second);

				TabOpenRequested (PlayerTC_.TabClass_);
			}
			else if (key == "artistbrowser")
			{
				QString artist;
				stream >> artist;
				new ArtistBrowserTab { artist, recInfo.DynProperties_ };
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown context"
						<< recInfo.Data_;
		}
	}

	bool Plugin::HasSimilarTab (const QByteArray& data, const QList<QByteArray>& others) const
	{
		return StandardSimilarImpl (data, others,
				[] (const QByteArray& data) { return data; });
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.General";
		result << "org.LeechCraft.LMP.CollectionSync";
		result << "org.LeechCraft.LMP.CloudStorage";
		result << "org.LeechCraft.LMP.PlaylistProvider";
		result << "org.LeechCraft.LMP.FiltersProvider";
		return result;
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);

		if (const auto ifp = qobject_cast<IFilterPlugin*> (plugin))
			for (const auto& effect : ifp->GetEffects ())
				EffectsMgr_->RegisterEffect (effect);
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return Core::Instance ().GetProgressManager ()->GetModel ();
	}

	QString Plugin::GetFilterVerb () const
	{
		return tr ("Show artist information");
	}

	QList<IDataFilter::FilterVariant> Plugin::GetFilterVariants (const QVariant&) const
	{
		return
		{
			{
				GetUniqueID () + ".ArtistBrowser",
				tr ("Show artist information"),
				tr ("Search for artist biography, similar artists, releases and so on."),
				ArtistBrowserTab::GetStaticTabClass ().Icon_
			}
		};
	}

	QString Plugin::GetDiagInfoString () const
	{
		return CollectDiagInfo ();
	}

	void Plugin::InitShortcuts ()
	{
		Entity e = Util::MakeEntity ({}, {}, {},
				Mimes::GlobalActionRegister);
		e.Additional_ [EF::GlobalAction::Receiver] = QVariant::fromValue<QObject*> (PlayerTab_->GetPlayer ());
		auto initShortcut = [&e, this] (const QByteArray& method, const QKeySequence& seq)
		{
			Entity thisE = e;
			thisE.Additional_ [EF::GlobalAction::ActionID] = "LMP_Global_" + method;
			thisE.Additional_ [EF::GlobalAction::Method] = method;
			thisE.Additional_ [EF::GlobalAction::Shortcut] = QVariant::fromValue (seq);
			GlobAction2Entity_ ["LMP_Global_" + method] = thisE;
		};
		initShortcut (SLOT (togglePause ()), QStringLiteral ("Meta+C"));
		initShortcut (SLOT (previousTrack ()), QStringLiteral ("Meta+V"));
		initShortcut (SLOT (nextTrack ()), QStringLiteral ("Meta+B"));
		initShortcut (SLOT (stop ()), QStringLiteral ("Meta+X"));
		initShortcut (SLOT (stopAfterCurrent ()), QStringLiteral ("Meta+Alt+X"));

		auto output = PlayerTab_->GetPlayer ()->GetAudioOutput ();
		auto controller = new VolumeNotifyController (output, PlayerTab_->GetPlayer ());
		e.Additional_ [EF::GlobalAction::Receiver] = QVariant::fromValue<QObject*> (controller);
		initShortcut (SLOT (volumeUp ()), {});
		initShortcut (SLOT (volumeDown ()), {});

		e.Additional_ [EF::GlobalAction::Receiver] = QVariant::fromValue<QObject*> (PlayerTab_);
		initShortcut (SLOT (handleLoveTrack ()), QStringLiteral ("Meta+L"));
		initShortcut (SIGNAL (notifyCurrentTrackRequested ()), {});

		auto proxy = GetProxyHolder ();
		auto setInfo = [this, proxy] (const QByteArray& method,
				const QString& userText, const QString& icon)
		{
			const auto& id = "LMP_Global_" + method;
			const auto& seq = GlobAction2Entity_ [id].Additional_ [EF::GlobalAction::Shortcut].value<QKeySequence> ();
			GlobAction2Info_ [id] = { userText, seq, proxy->GetIconThemeManager ()->GetIcon (icon) };
		};
		setInfo (SLOT (togglePause ()), tr ("Play/pause"), QStringLiteral ("media-playback-start"));
		setInfo (SLOT (previousTrack ()), tr ("Previous track"), QStringLiteral ("media-skip-backward"));
		setInfo (SLOT (nextTrack ()), tr ("Next track"), QStringLiteral ("media-skip-forward"));
		setInfo (SLOT (stop ()), tr ("Stop playback"), QStringLiteral ("media-playback-stop"));
		setInfo (SLOT (stopAfterCurrent ()), tr ("Stop playback after current track"), QStringLiteral ("process-stop"));
		setInfo (SLOT (handleLoveTrack ()), tr ("Love track"), QStringLiteral ("emblem-favorite"));
		setInfo (SIGNAL (notifyCurrentTrackRequested ()),
				tr ("Notify about current track"),
				QStringLiteral ("dialog-information"));
		setInfo (SLOT (volumeUp ()), tr ("Increase volume"), QStringLiteral ("audio-volume-high"));
		setInfo (SLOT (volumeDown ()), tr ("Decrease volume"), QStringLiteral ("audio-volume-low"));
	}

	void Plugin::handleFullRaiseRequested ()
	{
		TabOpenRequested (PlayerTC_.TabClass_);
	}

	void Plugin::showCollectionStats ()
	{
		auto dia = new CollectionStatsDialog ();
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp, LC::LMP::Plugin);
