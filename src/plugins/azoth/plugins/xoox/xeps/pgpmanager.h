/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QByteArray>
#include <QtCrypto>
#include <QXmppClientExtension.h>
#include <QXmppMessage.h>
#include <QXmppPresence.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PgpManager : public QXmppClientExtension
	{
		Q_OBJECT

		// private key, used for decrypting messages
		QCA::PGPKey PrivateKey_;

		// map of userIDs and corresponding public keys
		// each user ID is a completely arbitrary value, one can use JIDs for this purpose
		QMap<QString, QCA::PGPKey> PublicKeys_;
	public:
		QCA::PGPKey PublicKey (const QString&) const;
		void SetPublicKey (const QString&, const QCA::PGPKey&);

		QCA::PGPKey PrivateKey () const;
		void SetPrivateKey (const QCA::PGPKey&);

		QByteArray EncryptBody (const QCA::PGPKey&, const QByteArray&);
		QByteArray SignMessage (const QByteArray&);
		QByteArray SignPresence (const QByteArray&);

		QByteArray DecryptBody (const QByteArray&);
		bool IsValidSignature (const QCA::PGPKey&, const QByteArray&, const QByteArray&);

		bool handleStanza (const QDomElement&);
	signals:
		void encryptedMessageReceived (const QString&, const QString&);
		void signedMessageReceived (const QString&);
		void signedPresenceReceived (const QString&);
		void invalidSignatureReceived (const QString&);
	};
}
}
}
