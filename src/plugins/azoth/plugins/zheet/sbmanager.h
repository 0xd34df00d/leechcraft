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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_SBMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_SBMANAGER_H
#include <QObject>
#include <QHash>
#include <QSet>
#include <msn/util.h>

namespace MSN
{
	class SwitchboardServerConnection;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNAccount;
	class MSNMessage;
	class MSNBuddyEntry;
	class Callbacks;

	class SBManager : public QObject
	{
		Q_OBJECT

		MSNAccount *Account_;
		Callbacks *CB_;

		QHash<const MSNBuddyEntry*, QList<MSNMessage*>> PendingMessages_;
		QHash<const MSNBuddyEntry*, QList<MSN::fileTransferInvite>> PendingTransfers_;
		QSet<const MSNBuddyEntry*> PendingNudges_;
		QHash<const MSNBuddyEntry*, MSN::SwitchboardServerConnection*> Switchboards_;

		QHash<int, MSNMessage*> PendingDelivery_;
	public:
		SBManager (Callbacks*, MSNAccount*);

		void SendMessage (MSNMessage*, const MSNBuddyEntry*);
		void SendNudge (const QString&, const MSNBuddyEntry*);
		void SendFile (const QString&, uint, const MSNBuddyEntry*);
	private slots:
		void handleGotSB (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*);
		void handleBuddyJoined (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*);
		void handleBuddyLeft (MSN::SwitchboardServerConnection*, const MSNBuddyEntry*);
		void handleMessageDelivered (int);
	};
}
}
}

#endif
