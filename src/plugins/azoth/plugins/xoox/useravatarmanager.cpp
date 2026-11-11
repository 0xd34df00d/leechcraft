/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "useravatarmanager.h"
#include <QCryptographicHash>
#include <util/threads/coro.h>
#include <interfaces/azoth/iproxyobject.h>
#include "xeps/pubsubmanager.h"
#include "useravatardata.h"
#include "useravatarmetadata.h"
#include "clientconnection.h"
#include "interfaces/azoth/core/iavatarsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	UserAvatarManager::UserAvatarManager (IAvatarsManager *avatarsMgr, ClientConnection *conn)
	: QObject { conn }
	, Manager_ { conn->GetPubSubManager () }
	, Conn_ { conn }
	, AvatarsMgr_ { avatarsMgr }
	{
		connect (Manager_,
				SIGNAL (gotEvent (QString, PEPEventBase*)),
				this,
				SLOT (handleEvent (QString, PEPEventBase*)));

		Manager_->RegisterCreator<UserAvatarData> ();
		Manager_->RegisterCreator<UserAvatarMetadata> ();
		Manager_->SetAutosubscribe<UserAvatarMetadata> (true);
	}

	void UserAvatarManager::PublishAvatar (const QImage& avatar)
	{
		if (!avatar.isNull ())
		{
			UserAvatarData data (avatar);

			Manager_->PublishEvent (&data);
		}

		UserAvatarMetadata metadata (avatar);
		Manager_->PublishEvent (&metadata);
	}

	Util::ContextTask<void> UserAvatarManager::HandleMDEvent (QString from, UserAvatarMetadata *mdEvent)
	{
		const auto entry = qobject_cast<ICLEntry*> (Conn_->GetCLEntry (from));
		if (!entry)
			co_return;

		const auto id = mdEvent->GetId ();
		if (id.isEmpty ())
		{
			emit avatarUpdated (from);
			co_return;
		}

		co_await Util::AddContext { Util::CoroContext::Of (*this) };

		const auto data = co_await AvatarsMgr_->GetLocalAvatarData (entry->GetGlobalStrongestId ());
		if (!data || data->isEmpty () || QCryptographicHash::hash (*data, QCryptographicHash::Sha1).toHex () != id)
			emit avatarUpdated (from);
	}

	void UserAvatarManager::handleEvent (const QString& from, PEPEventBase *event)
	{
		if (auto mdEvent = dynamic_cast<UserAvatarMetadata*> (event))
			HandleMDEvent (from, mdEvent);
	}
}
}
}
