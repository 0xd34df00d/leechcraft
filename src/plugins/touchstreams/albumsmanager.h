/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/util.h>

class QStandardItem;

namespace LC
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}

enum class QueuePriority;
}

namespace TouchStreams
{
	class AlbumsManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		const qlonglong UserID_ = -1;

		Util::SvcAuth::VkAuthManager * const AuthMgr_;
		QList<QPair<std::function<void (QString)>, Util::QueuePriority>> RequestQueue_;
		const Util::DefaultScopeGuard RequestQueueGuard_;

		struct AlbumInfo
		{
			qlonglong ID_ = static_cast<qlonglong> (-1);
			QString Name_;
			QStandardItem *Item_ = nullptr;

			AlbumInfo () = default;
			AlbumInfo (qlonglong, const QString&, QStandardItem*);
		};
		QHash<qlonglong, AlbumInfo> Albums_;

		QStandardItem * const AlbumsRootItem_;

		quint32 TracksCount_ = 0;
	public:
		AlbumsManager (Util::SvcAuth::VkAuthManager*, ICoreProxy_ptr, QObject* = 0);
		AlbumsManager (qlonglong, Util::SvcAuth::VkAuthManager*, ICoreProxy_ptr, QObject* = 0);
		AlbumsManager (qlonglong, const QVariant& albums, const QVariant& tracks,
				Util::SvcAuth::VkAuthManager*, ICoreProxy_ptr, QObject* = 0);

		QStandardItem* GetRootItem () const;
		qlonglong GetUserID () const;
		quint32 GetTracksCount () const;

		void RefreshItems (QList<QStandardItem*>&);
	private:
		void InitRootItem ();

		bool HandleAlbums (const QVariant&);
		bool HandleTracks (const QVariant&);
	public slots:
		void refetchAlbums ();
		void handleAlbumsFetched ();
		void handleTracksFetched ();
	signals:
		void finished (AlbumsManager*);
	};
}
}
