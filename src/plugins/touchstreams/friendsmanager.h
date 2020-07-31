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
#include <QVariantMap>
#include <util/sll/util.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;

namespace LC
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}

class QueueManager;
}

namespace TouchStreams
{
	class AlbumsManager;
	class RecsManager;

	class FriendsManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		Util::SvcAuth::VkAuthManager * const AuthMgr_;
		Util::QueueManager * const Queue_;
		QList<std::function<void (QString)>> RequestQueue_;
		const Util::DefaultScopeGuard RequestQueueGuard_;

		QHash<qulonglong, QStandardItem*> Friend2Item_;
		QHash<qulonglong, std::shared_ptr<AlbumsManager>> Friend2AlbumsManager_;
		QHash<qulonglong, std::shared_ptr<RecsManager>> Friend2RecsManager_;

		QStandardItem *Root_;

		QHash<QNetworkReply*, QMap<qlonglong, QVariantMap>> Reply2Users_;

		typedef std::function<QNetworkReply* (QMap<QString, QString>)> ReplyMaker_f;
		QHash<QNetworkReply*, ReplyMaker_f> Reply2Func_;
		ReplyMaker_f CaptchaReplyMaker_;
	public:
		FriendsManager (Util::SvcAuth::VkAuthManager*, Util::QueueManager*, ICoreProxy_ptr, QObject* = 0);

		QStandardItem* GetRootItem () const;

		void RefreshItems (QList<QStandardItem*>&);
	private:
		void ScheduleTracksRequests (const QList<qlonglong>&, const QMap<qlonglong, QVariantMap>&);
		void ShowFriendsList (const QList<qlonglong>&, const QMap<qlonglong, QVariantMap>&);
		void MakeFriendItem (qlonglong id, const QVariantMap& userInfo,
				const QVariant& albums, const QVariant& tracks);
	private slots:
		void refetchFriends ();
		void handleGotFriends ();

		void handleCaptchaEntered (const QString&, const QString&);
		void handleExecuted ();

		void handleAlbumsFinished (AlbumsManager*);
		void handlePhotoFetched ();
	};
}
}
