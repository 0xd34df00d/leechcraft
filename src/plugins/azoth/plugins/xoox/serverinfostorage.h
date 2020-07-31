/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QXmppDiscoveryIq.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class AccountSettingsHolder;
	class ClientConnection;

	class ServerInfoStorage : public QObject
	{
		Q_OBJECT

		ClientConnection *Conn_;
		AccountSettingsHolder *Settings_;
		QString PreviousJID_;
		QString Server_;

		QStringList ServerFeatures_;
		QStringList SelfJIDFeatures_;

		QString BytestreamsProxy_;
	public:
		ServerInfoStorage (ClientConnection*, AccountSettingsHolder*);

		bool HasServerFeatures () const;
		QStringList GetServerFeatures () const;

		bool HasSelfFeatures () const;
		QStringList GetSelfFeatures () const;

		QString GetBytestreamsProxy () const;
	private:
		void HandleItems (const QXmppDiscoveryIq&);
		void HandleItemInfo (const QXmppDiscoveryIq&);
	private slots:
		void handleConnected ();
	signals:
		void bytestreamsProxyChanged (const QString&);
	};
}
}
}
