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

class QXmppDiscoveryManager;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class CapsDatabase;

	class CapsManager : public QObject
	{
		Q_OBJECT

		ClientConnection *Connection_;
		QXmppDiscoveryManager *DiscoMgr_;
		CapsDatabase *DB_;
		QHash<QString, QString> Caps2String_;
	public:
		CapsManager (QXmppDiscoveryManager*, ClientConnection*, CapsDatabase*);

		void FetchCaps (const QString&, const QByteArray&);
		QStringList GetRawCaps (const QByteArray&) const;
		QStringList GetCaps (const QByteArray&) const;
		QStringList GetCaps (const QStringList&) const;

		void SetIdentities (const QByteArray&, const QList<QXmppDiscoveryIq::Identity>&);
		QList<QXmppDiscoveryIq::Identity> GetIdentities (const QByteArray&) const;
	public slots:
		void handleInfoReceived (const QXmppDiscoveryIq&);
		void handleItemsReceived (const QXmppDiscoveryIq&);
	};
}
}
}
