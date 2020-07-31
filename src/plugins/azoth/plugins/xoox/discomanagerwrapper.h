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

class QXmppDiscoveryManager;
class QXmppDiscoveryIq;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	class DiscoManagerWrapper : public QObject
	{
		Q_OBJECT

	public:
		typedef std::function<void (const QXmppDiscoveryIq&)> DiscoCallback_t;
	private:
		QXmppDiscoveryManager * const Mgr_;
		ClientConnection * const Conn_;

		QHash<QString, DiscoCallback_t> AwaitingDiscoInfo_;
		QHash<QString, DiscoCallback_t> AwaitingDiscoItems_;
	public:
		DiscoManagerWrapper (QXmppDiscoveryManager*, ClientConnection*);

		void RequestInfo (const QString&, DiscoCallback_t, bool reportErrors = false, const QString& = "");
		void RequestItems (const QString&, DiscoCallback_t, bool reportErrors = false, const QString& = "");
	private slots:
		void handleDiscoInfo (const QXmppDiscoveryIq&);
		void handleDiscoItems (const QXmppDiscoveryIq&);
	};
}
}
}
