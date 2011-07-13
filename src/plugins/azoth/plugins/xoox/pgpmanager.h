/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

class QXmppAnnotationsManager;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class QXmppPgpManager : public QXmppClientExtension
	{
		Q_OBJECT

	public:
		QCA::PGPKey publicKey (const QString&) const;
		void setPublicKey (const QString&, const QCA::PGPKey&);

		QCA::PGPKey privateKey () const;
		void setPrivateKey (const QCA::PGPKey&);

		bool encryptBody (const QCA::PGPKey&, const QByteArray&, QByteArray&);
		bool signMessage (const QByteArray&, QByteArray&);
		bool signPresence (const QByteArray&, QByteArray&);

		bool decryptBody (const QByteArray&, QByteArray&);
		bool isValidSignature (const QCA::PGPKey&, const QByteArray&, const QByteArray&);

		bool handleStanza (const QDomElement &element);

	signals:
		void encryptedMessageReceived (const QXmppMessage&);
		void signedMessageReceived (const QXmppMessage&);
		void signedPresenceReceived (const QXmppPresence&);
		void invalidSignatureReceived (const QDomElement &);

	private:
		// set presence type enum value from its string representation
		QXmppPresence::Type setPresenceTypeFromStr (const QString&);
		// private key, used for decrypting messages
		QCA::PGPKey PrivateKey_;
		// map of userIDs and corresponding public keys
		// each user ID is a completely arbitrary value, one can use JIDs for this purpose
		QMap<QString, QCA::PGPKey> PublicKeys_;
	};
}
}
}

#endif
