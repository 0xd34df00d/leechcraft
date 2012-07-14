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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_PGPMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_PGPMANAGER_H
#include <QMap>
#include <QByteArray>
#include <QtCrypto>
#include <QXmppClientExtension.h>
#include <QXmppMessage.h>
#include <QXmppPresence.h>

namespace LeechCraft
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

#endif
