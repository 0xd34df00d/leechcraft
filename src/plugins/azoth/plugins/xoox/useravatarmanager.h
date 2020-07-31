/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
class IAvatarsManager;

namespace Xoox
{
	class PubSubManager;
	class PEPEventBase;
	class ClientConnection;
	class UserAvatarMetadata;

	class UserAvatarManager : public QObject
	{
		Q_OBJECT

		PubSubManager * const Manager_;
		ClientConnection * const Conn_;
		IAvatarsManager * const AvatarsMgr_;
	public:
		UserAvatarManager (IAvatarsManager*, ClientConnection*);

		void PublishAvatar (const QImage&);
	private:
		void HandleMDEvent (const QString&, UserAvatarMetadata*);
	private slots:
		void handleEvent (const QString&, PEPEventBase*);
	signals:
		void avatarUpdated (const QString&);
	};
}
}
}
