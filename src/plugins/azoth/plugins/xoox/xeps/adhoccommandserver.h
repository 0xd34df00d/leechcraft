/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QSet>
#include <QXmppClientExtension.h>
#include <QXmppDataForm.h>
#include <QXmppDiscoveryIq.h>

class QXmppDiscoveryIq;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Xoox
{
	class ClientConnection;

	class AdHocCommandServer : public QXmppClientExtension
	{
		Q_OBJECT

		ClientConnection * const Conn_;
		IProxyObject * const Proxy_;

		QMap<QString, QXmppDiscoveryIq::Item> XEP0146Items_;

		typedef std::function<void (const QDomElement&)> NodeActor_t;
		QMap<QString, NodeActor_t> NodeInfos_;
		typedef std::function<void (const QDomElement&,
				const QString&, const QXmppDataForm&)> NodeSubmitHandler_t;
		QMap<QString, NodeSubmitHandler_t> NodeSubmitHandlers_;

		QMap<QString, QStringList> PendingSessions_;
	public:
		AdHocCommandServer (ClientConnection*, IProxyObject*);

		bool handleStanza (const QDomElement&);
	private:
		bool HandleDiscoIq (const QDomElement&);
		bool HandleIqSet (const QDomElement&);

		void Send (const QXmppDataForm&, const QDomElement&, const QString&);
		void SendCompleted (const QDomElement&, const QString&, const QString&);

		void ChangeStatusInfo (const QDomElement&);
		void ChangeStatusSubmitted (const QDomElement&,
				const QString&, const QXmppDataForm&);
		void LeaveGroupchatsInfo (const QDomElement&);
		void LeaveGroupchatsSubmitted (const QDomElement&,
				const QString&, const QXmppDataForm&);
		void Forward (const QDomElement&);
		void AddTaskInfo (const QDomElement&);
		void AddTaskSubmitted (const QDomElement&,
				const QString&, const QXmppDataForm&);
	};
}
}
}
