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

#pragma once

#include <QObject>
#include <QSet>
#include <QHash>

#ifdef ENABLE_CRYPT
#include <QtCrypto>
#endif

class QXmppMessage;
class QXmppPresence;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class GlooxMessage;

#ifdef ENABLE_CRYPT
	class PgpManager;
#endif

	class CryptHandler : public QObject
	{
		Q_OBJECT

		ClientConnection *Conn_;

#ifdef ENABLE_CRYPT
		PgpManager *PGPManager_;
#endif

		QSet<QString> SignedPresences_;
		QSet<QString> SignedMessages_;
		QHash<QString, QString> EncryptedMessages_;
		QSet<QString> Entries2Crypt_;
	public:
		CryptHandler (ClientConnection*);

		void Init ();

		void HandlePresence (const QXmppPresence&, const QString&, const QString&);
		void ProcessOutgoing (QXmppMessage&, GlooxMessage*);
		void ProcessIncoming (QXmppMessage&);

#ifdef ENABLE_CRYPT
		PgpManager* GetPGPManager () const;
		bool SetEncryptionEnabled (const QString&, bool);
#endif
	private slots:
		void handleEncryptedMessageReceived (const QString&, const QString&);
		void handleSignedMessageReceived (const QString&);
		void handleSignedPresenceReceived (const QString&);
		void handleInvalidSignatureReceived (const QString&);
	};
}
}
}
