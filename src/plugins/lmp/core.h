/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QObject>

class QUrl;

namespace LC
{
struct Entity;

namespace LMP
{
	struct MediaInfo;
	class LocalCollection;
	class HookInterconnector;
	class LocalFileResolver;
	class PlaylistManager;
	class SyncManager;
	class SyncUnmountableManager;
	class CloudUploadManager;
	class Player;
	class ProgressManager;
	class RadioManager;
	class CollectionsManager;
	class LMPProxy;

	class Core : public QObject
	{
		Q_OBJECT

		static std::shared_ptr<Core> CoreInstance_;

		struct Members;

		std::shared_ptr<Members> M_;

		QObjectList SyncPlugins_;
		QObjectList CloudPlugins_;

		Core ();
		Core (const Core&) = delete;
		Core (Core&&) = delete;
		Core& operator= (const Core&) = delete;
		Core& operator= (Core&&) = delete;
	public:
		static Core& Instance ();
		static void InitWithProxy ();

		void Release ();

		void InitWithOtherPlugins ();

		LMPProxy* GetLmpProxy () const;

		void AddPlugin (QObject*);
		QObjectList GetSyncPlugins () const;
		QObjectList GetCloudStoragePlugins () const;

		HookInterconnector* GetHookInterconnector () const;
		LocalFileResolver* GetLocalFileResolver () const;
		LocalCollection* GetLocalCollection () const;
		CollectionsManager* GetCollectionsManager () const;
		PlaylistManager* GetPlaylistManager () const;
		SyncManager* GetSyncManager () const;
		SyncUnmountableManager* GetSyncUnmountableManager () const;
		CloudUploadManager* GetCloudUploadManager () const;
		ProgressManager* GetProgressManager () const;
		RadioManager* GetRadioManager () const;

		Player* GetPlayer () const;

		std::optional<MediaInfo> TryURLResolve (const QUrl&) const;
	public slots:
		void rescan ();
	signals:
		void cloudStoragePluginsChanged ();
	};
}
}
