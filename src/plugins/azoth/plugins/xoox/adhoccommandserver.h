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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ADHOCCOMMANDSERVER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ADHOCCOMMANDSERVER_H
#include <functional>
#include <QSet>
#include <QXmppClientExtension.h>
#include <QXmppDataForm.h>
#include <QXmppDiscoveryIq.h>

class QXmppDiscoveryIq;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	class AdHocCommandServer : public QXmppClientExtension
	{
		Q_OBJECT

		ClientConnection *Conn_;

		QMap<QString, QXmppDiscoveryIq::Item> XEP0146Items_;

		typedef std::function<void (const QDomElement&)> NodeActor_t;
		QMap<QString, NodeActor_t> NodeInfos_;
		typedef std::function<void (const QDomElement&,
				const QString&, const QXmppDataForm&)> NodeSubmitHandler_t;
		QMap<QString, NodeSubmitHandler_t> NodeSubmitHandlers_;

		QMap<QString, QStringList> PendingSessions_;
	public:
		AdHocCommandServer (ClientConnection*);

		bool handleStanza (const QDomElement&);
	private:
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
	private slots:
		void handleDiscoItems (const QXmppDiscoveryIq&);
		void handleDiscoInfo (const QXmppDiscoveryIq&);
	};
}
}
}

#endif
