/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <interfaces/iplugin2.h>
#include <interfaces/core/ientitymanager.h>
#include "collectionsmanager.h"
#include "localfileresolver.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"
#include "playlistmanager.h"
#include "sync/syncmanager.h"
#include "sync/syncunmountablemanager.h"
#include "sync/clouduploadmanager.h"
#include "interfaces/lmp/ilmpplugin.h"
#include "interfaces/lmp/icloudstorageplugin.h"
#include "lmpproxy.h"
#include "player.h"
#include "progressmanager.h"
#include "radiomanager.h"
#include "rganalysismanager.h"
#include "hookinterconnector.h"

namespace LC
{
namespace LMP
{
	std::shared_ptr<Core> Core::CoreInstance_;

	struct Core::Members
	{
		LocalFileResolver Resolver_;

		HookInterconnector HookInterconnector_;

		LocalCollection Collection_;
		CollectionsManager CollectionsManager_;

		PlaylistManager PLManager_;

		SyncManager SyncManager_;
		SyncUnmountableManager SyncUnmountableManager_;
		CloudUploadManager CloudUpMgr_;

		ProgressManager ProgressManager_;

		RadioManager RadioManager_;

		Player Player_;

		LMPProxy LmpProxy_ { &Collection_, &Resolver_ };

		RgAnalysisManager RgMgr_ { &Collection_ };

		Members () = default;
	};

	Core::Core ()
	: M_ (std::make_shared<Members> ())
	{
		M_->ProgressManager_.AddSyncManager (&M_->SyncManager_);
		M_->ProgressManager_.AddSyncManager (&M_->SyncUnmountableManager_);
		M_->ProgressManager_.AddSyncManager (&M_->CloudUpMgr_);

		M_->CollectionsManager_.SetCollectionModel (M_->Collection_.GetCollectionModel ());
	}

	Core& Core::Instance ()
	{
		return *CoreInstance_;
	}

	void Core::InitWithProxy ()
	{
		CoreInstance_.reset (new Core {});
	}

	void Core::Release ()
	{
		CoreInstance_.reset ();
	}

	void Core::InitWithOtherPlugins ()
	{
		M_->Player_.InitWithOtherPlugins ();
		M_->RadioManager_.InitProviders ();
	}

	LMPProxy* Core::GetLmpProxy () const
	{
		return &M_->LmpProxy_;
	}

	void Core::AddPlugin (QObject *pluginObj)
	{
		auto ip2 = qobject_cast<IPlugin2*> (pluginObj);
		auto ilmpPlug = qobject_cast<ILMPPlugin*> (pluginObj);

		if (!ilmpPlug)
		{
			qWarning () << Q_FUNC_INFO
					<< pluginObj
					<< "doesn't implement ILMPPlugin";
			return;
		}

		ilmpPlug->SetLMPProxy (&M_->LmpProxy_);

		const auto& classes = ip2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.LMP.CollectionSync") &&
			qobject_cast<ISyncPlugin*> (pluginObj))
			SyncPlugins_ << pluginObj;

		if (classes.contains ("org.LeechCraft.LMP.CloudStorage") &&
			qobject_cast<ICloudStoragePlugin*> (pluginObj))
		{
			CloudPlugins_ << pluginObj;
			emit cloudStoragePluginsChanged ();
		}

		if (classes.contains ("org.LeechCraft.LMP.PlaylistProvider") &&
			qobject_cast<IPlaylistProvider*> (pluginObj))
			M_->PLManager_.AddProvider (pluginObj);

		M_->HookInterconnector_.AddPlugin (pluginObj);
	}

	QObjectList Core::GetSyncPlugins () const
	{
		return SyncPlugins_;
	}

	QObjectList Core::GetCloudStoragePlugins () const
	{
		return CloudPlugins_;
	}

	HookInterconnector* Core::GetHookInterconnector () const
	{
		return &M_->HookInterconnector_;
	}

	LocalFileResolver* Core::GetLocalFileResolver () const
	{
		return &M_->Resolver_;
	}

	LocalCollection* Core::GetLocalCollection () const
	{
		return &M_->Collection_;
	}

	CollectionsManager* Core::GetCollectionsManager () const
	{
		return &M_->CollectionsManager_;
	}

	PlaylistManager* Core::GetPlaylistManager () const
	{
		return &M_->PLManager_;
	}

	SyncManager* Core::GetSyncManager () const
	{
		return &M_->SyncManager_;
	}

	SyncUnmountableManager* Core::GetSyncUnmountableManager () const
	{
		return &M_->SyncUnmountableManager_;
	}

	CloudUploadManager* Core::GetCloudUploadManager () const
	{
		return &M_->CloudUpMgr_;
	}

	ProgressManager* Core::GetProgressManager () const
	{
		return &M_->ProgressManager_;
	}

	RadioManager* Core::GetRadioManager () const
	{
		return &M_->RadioManager_;
	}

	Player* Core::GetPlayer () const
	{
		return &M_->Player_;
	}

	std::optional<MediaInfo> Core::TryURLResolve (const QUrl& url) const
	{
		return M_->PLManager_.TryResolveMediaInfo (url);
	}

	void Core::rescan ()
	{
		M_->Collection_.Rescan ();
	}
}
}
