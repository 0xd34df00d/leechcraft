/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
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
		PgpManager *PGPManager_ = nullptr;
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
		bool IsEncryptionEnabled (const QString&) const;
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
