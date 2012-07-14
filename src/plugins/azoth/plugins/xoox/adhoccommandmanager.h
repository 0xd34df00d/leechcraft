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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ADHOCCOMMANDMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ADHOCCOMMANDMANAGER_H
#include <QSet>
#include <QXmppClientExtension.h>
#include <QXmppDataForm.h>
#include "adhoccommand.h"

class QXmppDiscoveryIq;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	class AdHocCommandManager : public QXmppClientExtension
	{
		Q_OBJECT
		
		ClientConnection *ClientConn_;
		QSet<QString> PendingCommands_;
	public:
		static QString GetAdHocFeature ();

		AdHocCommandManager (ClientConnection*);

		void QueryCommands (const QString&);
		void ExecuteCommand (const QString&, const AdHocCommand&);
		void ProceedExecuting (const QString&, const AdHocResult&, const QString&);

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);
	private slots:
		void handleItemsReceived (const QXmppDiscoveryIq&);
	signals:
		void gotCommands (const QString&, const QList<AdHocCommand>&);
		void gotResult (const QString&, const AdHocResult&);
	};
}
}
}

#endif
