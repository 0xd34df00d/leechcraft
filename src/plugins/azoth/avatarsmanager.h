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
#include <util/sll/util.h>
#include "interfaces/azoth/ihaveavatars.h"
#include "interfaces/azoth/iproxyobject.h"

template<typename>
class QFuture;

namespace LC
{
namespace Azoth
{
	class AvatarsStorage;
	class IAccount;

	class AvatarsManager : public QObject
						 , public IAvatarsManager
	{
		Q_OBJECT

		AvatarsStorage * const Storage_;

		QHash<QObject*, QHash<IHaveAvatars::Size, QFuture<QImage>>> PendingRequests_;
	public:
		using AvatarHandler_f = std::function<void (QImage)>;
	private:
		uint64_t SubscriptionID_ = 0;
		QHash<QObject*, QHash<IHaveAvatars::Size, QHash<uint64_t, AvatarHandler_f>>> Subscriptions_;

		QHash<QObject*, const IAccount*> SelfInfo2Account_;
	public:
		explicit AvatarsManager (QObject* = nullptr);

		QFuture<QImage> GetAvatar (QObject*, IHaveAvatars::Size) override;
		QFuture<std::optional<QByteArray>> GetStoredAvatarData (const QString&, IHaveAvatars::Size) override;

		bool HasAvatar (QObject*) const;

		Util::DefaultScopeGuard Subscribe (QObject*, IHaveAvatars::Size, const AvatarHandler_f&);
	private:
		void HandleSubscriptions (QObject*);
	public slots:
		void handleAccount (QObject*);
	private slots:
		void handleEntries (const QList<QObject*>&);
		void invalidateAvatar (QObject*);

		void handleCacheSizeChanged ();
	signals:
		void avatarInvalidated (QObject*);
		void accountAvatarInvalidated (const IAccount*);
	};
}
}
