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
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>
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

		Util::InstallTranslator ("lmp");

		FixGstPaths ();

		gint argc = 1;
		gchar *argvarr [] = { g_strdup ("leechcraft"), nullptr };
		gchar **argv = argvarr;
		gst_init (&argc, &argv);

		qRegisterMetaType<QList<QPair<QString, QUrl>>> ("QList<QPair<QString, QUrl>>");
		qRegisterMetaTypeStreamOperators<QList<QPair<QString, QUrl>>> ();

		qRegisterMetaType<SavedFilterInfo> ("LC::LMP::SavedFilterInfo");
		qRegisterMetaTypeStreamOperators<SavedFilterInfo> ();
		qRegisterMetaType<QList<SavedFilterInfo>> ("QList<LC::LMP::SavedFilterInfo>");
		qRegisterMetaTypeStreamOperators<QList<SavedFilterInfo>> ();

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

		ArtistBrowserTC_ =
		{
			GetUniqueID () + "_artistBrowser",
			tr ("Artist browser"),
			tr ("Allows one to browse information about different artists."),
			QIcon ("lcicons:/lmp/resources/images/lmp_artist_browser.svg"),
			35,
			TFSuggestOpening | TFOpenableByRequest
		};

		Core::Instance ().InitWithProxy ();

		auto mgr = new RootPathSettingsManager (this);
		XSD_->SetDataSource ("RootPathsView", mgr->GetModel ());

		PlayerTab_ = new PlayerTab (PlayerTC_, Core::Instance ().GetPlayer (), proxy, this);

		Core::Instance ().GetLmpProxy ()->GetGuiProxy ()->SetPlayerTab (PlayerTab_);

		connect (PlayerTab_,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (PlayerTab_,
				SIGNAL (changeTabName (QWidget*, QString)),
				this,
				SIGNAL (changeTabName (QWidget*, QString)));
		connect (PlayerTab_,
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (artistBrowseRequested (QString)),
				this,
				SLOT (handleArtistBrowseRequested (QString)));

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
	}

	void Plugin::SecondInit ()
	{
		for (const auto& e : GlobAction2Entity_)
			Proxy_->GetEntityManager ()->HandleEntity (e);

		Core::Instance ().InitWithOtherPlugins ();
		PlayerTab_->InitWithOtherPlugins ();

		EffectsMgr_->RegisteringFinished ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& sequences)
	{
		if (!GlobAction2Entity_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown id"
					<< id;
			return;
		}

		auto& e = GlobAction2Entity_ [id];
		e.Additional_ ["Shortcut"] = QVariant::fromValue (sequences.value (0));
		e.Additional_ ["AltShortcuts"] = Util::Map (sequences.mid (1),
				&QVariant::fromValue<QKeySequence>);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
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
			ArtistBrowserTC_
		};
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == PlayerTC_.TabClass_)
		{
			emit addNewTab ("LMP", PlayerTab_);
			emit raiseTab (PlayerTab_);
		}
		else if (tc == ArtistBrowserTC_.TabClass_)
			handleArtistBrowseRequested ({});
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
		if (e.Mime_ == "x-leechcraft/power-state-changed")
			return EntityTestHandleResult { EntityTestHandleResult::PHigh };

		if (e.Mime_ == "x-leechcraft/data-filter-request")
		{
			if (!e.Additional_ ["DataFilter"].toString ().startsWith (GetUniqueID ()))
				return {};

			if (e.Entity_.type () != QVariant::String)
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

		if (e.Mime_ == "x-leechcraft/power-state-changed")
		{
			if (e.Entity_ == "Sleeping")
			{
				player->SavePlayState (true);
				player->setPause ();
			}
			else if (e.Entity_ == "WokeUp")
			{
				player->RestorePlayState ();
				Core::Instance ().GetRadioManager ()->HandleWokeUp ();
			}

			return;
		}
		if (e.Mime_ == "x-leechcraft/data-filter-request")
		{
			handleArtistBrowseRequested (e.Entity_.toString ().trimmed ());
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
				handleArtistBrowseRequested (artist, recInfo.DynProperties_);
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
				ArtistBrowserTC_.Icon_
			}
		};
	}

	QString Plugin::GetDiagInfoString () const
	{
		return DiagInfoCollector {} ();
	}

	void Plugin::InitShortcuts ()
	{
		Entity e = Util::MakeEntity ({}, {}, {},
				"x-leechcraft/global-action-register");
		e.Additional_ ["Receiver"] = QVariant::fromValue<QObject*> (PlayerTab_->GetPlayer ());
		auto initShortcut = [&e, this] (const QByteArray& method, const QKeySequence& seq)
		{
			Entity thisE = e;
			thisE.Additional_ ["ActionID"] = "LMP_Global_" + method;
			thisE.Additional_ ["Method"] = method;
			thisE.Additional_ ["Shortcut"] = QVariant::fromValue (seq);
			GlobAction2Entity_ ["LMP_Global_" + method] = thisE;
		};
		initShortcut (SLOT (togglePause ()), QString ("Meta+C"));
		initShortcut (SLOT (previousTrack ()), QString ("Meta+V"));
		initShortcut (SLOT (nextTrack ()), QString ("Meta+B"));
		initShortcut (SLOT (stop ()), QString ("Meta+X"));
		initShortcut (SLOT (stopAfterCurrent ()), QString ("Meta+Alt+X"));

		auto output = PlayerTab_->GetPlayer ()->GetAudioOutput ();
		auto controller = new VolumeNotifyController (output, PlayerTab_->GetPlayer ());
		e.Additional_ ["Receiver"] = QVariant::fromValue<QObject*> (controller);
		initShortcut (SLOT (volumeUp ()), {});
		initShortcut (SLOT (volumeDown ()), {});

		e.Additional_ ["Receiver"] = QVariant::fromValue<QObject*> (PlayerTab_);
		initShortcut (SLOT (handleLoveTrack ()), QString ("Meta+L"));
		initShortcut (SIGNAL (notifyCurrentTrackRequested ()), {});

		auto proxy = GetProxyHolder ();
		auto setInfo = [this, proxy] (const QByteArray& method,
				const QString& userText, const QString& icon)
		{
			const auto& id = "LMP_Global_" + method;
			const auto& seq = GlobAction2Entity_ [id].Additional_ ["Shortcut"].value<QKeySequence> ();
			GlobAction2Info_ [id] = { userText, seq, proxy->GetIconThemeManager ()->GetIcon (icon) };
		};
		setInfo (SLOT (togglePause ()), tr ("Play/pause"), "media-playback-start");
		setInfo (SLOT (previousTrack ()), tr ("Previous track"), "media-skip-backward");
		setInfo (SLOT (nextTrack ()), tr ("Next track"), "media-skip-forward");
		setInfo (SLOT (stop ()), tr ("Stop playback"), "media-playback-stop");
		setInfo (SLOT (stopAfterCurrent ()), tr ("Stop playback after current track"), "process-stop");
		setInfo (SLOT (handleLoveTrack ()), tr ("Love track"), "emblem-favorite");
		setInfo (SIGNAL (notifyCurrentTrackRequested ()),
				tr ("Notify about current track"),
				"dialog-information");
		setInfo (SLOT (volumeUp ()), tr ("Increase volume"), "audio-volume-high");
		setInfo (SLOT (volumeDown ()), tr ("Decrease volume"), "audio-volume-low");
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

	void Plugin::handleArtistBrowseRequested (const QString& artist, const DynPropertiesList_t& props)
	{
		auto tab = new ArtistBrowserTab (ArtistBrowserTC_, this);

		for (const auto& pair : props)
			tab->setProperty (pair.first, pair.second);

		emit addNewTab (tr ("Artist browser"), tab);
		emit raiseTab (tab);

		connect (tab,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));

		if (!artist.isEmpty ())
			tab->Browse (artist);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp, LC::LMP::Plugin);
