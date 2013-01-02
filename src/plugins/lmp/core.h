/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <boost/optional.hpp>
#include <QObject>
#include <interfaces/core/icoreproxy.h>

class QUrl;

namespace LeechCraft
{
struct Entity;

namespace LMP
{
	struct MediaInfo;
	class LocalCollection;
	class LocalFileResolver;
	class PlaylistManager;
	class SyncManager;
	class SyncUnmountableManager;
	class CloudUploadManager;
	class Player;
	class PreviewHandler;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		LocalFileResolver *Resolver_;
		LocalCollection *Collection_;
		PlaylistManager *PLManager_;
		SyncManager *SyncManager_;
		SyncUnmountableManager *SyncUnmountableManager_;
		CloudUploadManager *CloudUpMgr_;

		Player *Player_;
		PreviewHandler *PreviewMgr_;

		QObjectList SyncPlugins_;
		QObjectList CloudPlugins_;

		Core ();
	public:
		static Core& Instance ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy ();

		void SendEntity (const Entity&);

		void PostInit ();

		void AddPlugin (QObject*);
		QObjectList GetSyncPlugins () const;
		QObjectList GetCloudStoragePlugins () const;

		LocalFileResolver* GetLocalFileResolver () const;
		LocalCollection* GetLocalCollection () const;
		PlaylistManager* GetPlaylistManager () const;
		SyncManager* GetSyncManager () const;
		SyncUnmountableManager* GetSyncUnmountableManager () const;
		CloudUploadManager* GetCloudUploadManager () const;

		Player* GetPlayer () const;
		PreviewHandler* GetPreviewHandler () const;

		boost::optional<MediaInfo> TryURLResolve (const QUrl&) const;
	public slots:
		void rescan ();
	signals:
		void gotEntity (const LeechCraft::Entity&);

		void cloudStoragePluginsChanged ();

		void artistBrowseRequested (const QString&);
	};
}
}
