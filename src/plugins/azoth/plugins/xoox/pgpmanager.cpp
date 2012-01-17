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

#include "pgpmanager.h"
#include <QDomElement>
#include <QtCrypto>
#include <QXmppConstants.h>
#include <QXmppClient.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const char *NsSigned = "jabber:x:signed";
	const char *NsEncrypted = "jabber:x:encrypted";

	QCA::PGPKey PgpManager::PublicKey (const QString& id) const
	{
		if (PublicKeys_.contains (id))
			return PublicKeys_.value (id);
		
		const QString& bare = id.left (id.indexOf ('/'));
		return PublicKeys_.value (bare);
	}

	void PgpManager::SetPublicKey (const QString& id, const QCA::PGPKey& publicKey)
	{
		PublicKeys_.insert (id, publicKey);
	}

	QCA::PGPKey PgpManager::PrivateKey () const
	{
		return PrivateKey_;
	}

	void PgpManager::SetPrivateKey (const QCA::PGPKey& privateKey)
	{
		PrivateKey_ = privateKey;
	}
	
	namespace
	{
		QString WrapPGP (const QString& str, bool sig)
		{
			const QString type = sig ? "SIGNATURE" : "MESSAGE";
			
			QString result;
			result += QString ("-----BEGIN PGP %1-----\n").arg (type);
			result += "Version: PGP\n\n";
			result += str + "\n";
			result += QString ("-----END PGP %1-----\n").arg (type);
			return result;
		}
	}

	QByteArray PgpManager::EncryptBody (const QCA::PGPKey& pubkey, const QByteArray& body)
	{
		if (pubkey.isNull ())
		{
			warning (QString ("Cannot encrypt: public key is null"));
			return QByteArray ();
		}

		QCA::SecureMessageKey msgKey;
		msgKey.setPGPPublicKey (pubkey);
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setRecipient (msgKey);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.startEncrypt ();
		msg.update (body);
		msg.end ();
		msg.waitForFinished ();

		if (msg.success ())
			return msg.read ();
		else
		{
			info (QString ("Error encrypting: " + msg.errorCode ()));
			return QByteArray ();
		}
	}

	QByteArray PgpManager::SignMessage (const QByteArray& body)
	{
		QCA::SecureMessageKey msgKey;
		if (PrivateKey_.isNull ())
		{
			warning (QString ("Cannot sign: private key is null"));
			return QByteArray ();
		}
		
		msgKey.setPGPSecretKey (PrivateKey_);
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.setSigner (msgKey);
		msg.startSign (QCA::SecureMessage::Detached);
		msg.update (body);
		msg.end ();
		msg.waitForFinished ();

		if (!msg.success ())
		{
			warning (QString ("Error signing: " + msg.errorCode ()));
			return QByteArray ();
		}
		
		const QByteArray& sig = msg.signature ();
		const QList<QByteArray>& arrs = sig.split ('\n');
		QList<QByteArray>::const_iterator it = arrs.begin ();
		++it;
		if (it == arrs.end ())
			return sig;
		
		for (; it != arrs.end (); ++it)
			if (it->isEmpty ())
				break;
			
		if (++it >= arrs.end ())
			return sig;
		
		QByteArray result;
		for (; it != arrs.end (); ++it)
		{
			if (it->at (0) == '-')
				break;
			result += *it;
			result += '\n';
		}
		result.chop (1);
		return result;
	}

	QByteArray PgpManager::SignPresence (const QByteArray& status)
	{
		QCA::SecureMessageKey msgKey;
		if (PrivateKey_.isNull ())
		{
			warning (QString ("Cannot sign: private key is null"));
			return QByteArray();
		}

		msgKey.setPGPSecretKey (PrivateKey_);
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.setSigner (msgKey);
		msg.startSign (QCA::SecureMessage::Detached);
		msg.update (status);
		msg.end ();
		msg.waitForFinished ();

		if (msg.success ())
			return msg.signature ();
		else
		{
			info (QString ("Error signing: " + msg.errorCode ()));
			return QByteArray ();
		}
	}

	QByteArray PgpManager::DecryptBody (const QByteArray& encrypted)
	{
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.startDecrypt ();
		msg.update (WrapPGP (encrypted, false).toUtf8 ());
		msg.end ();
		msg.waitForFinished ();

		if (msg.success ())
			return msg.read ();
		else
		{
			info (QString ("Error decrypting: " + msg.errorCode ()));
			return QByteArray ();
		}
	}

	bool PgpManager::IsValidSignature (const QCA::PGPKey& pubkey,
			const QByteArray& message, const QByteArray& signature)
	{
		if (pubkey.isNull ())
		{
			warning (QString ("Cannot encrypt: public key is null"));
			return false;
		}
		
		QCA::OpenPGP pgp;
		QCA::SecureMessageKey key;
		QCA::SecureMessage msg (&pgp);
		key.setPGPPublicKey (pubkey);
		msg.setSigner (key);
		msg.setFormat (QCA::SecureMessage::Binary);
		msg.startVerify (WrapPGP (signature, true).toUtf8 ());
		msg.update (message);
		msg.end ();
		msg.waitForFinished ();

		if (msg.verifySuccess ())
			return true;
		else
		{
			info (QString ("Invalid signature: " + msg.errorCode ()));
			return false;
		}
	}

	bool PgpManager::handleStanza (const QDomElement& stanza)
	{
		const QString& tagName = stanza.tagName ();
		if (("message" != tagName) && ("presence" != tagName))
			return false;

		const QString& from = stanza.attribute ("from");

		// Case 1: signed presence|message
		const QDomElement& x_element = stanza.firstChildElement ("x");
		if (x_element.namespaceURI () == NsSigned)
		{
			const QDomElement& status = stanza.firstChildElement ("status");
			QString message = status.text ();
			QString signature = x_element.text ();

			const QCA::PGPKey key = PublicKey (from);

			if (!IsValidSignature (key, message.toUtf8 (), signature.toAscii ()))
				emit invalidSignatureReceived (from);
			else if (tagName == "message")
				emit signedMessageReceived (from);
			else if (tagName == "presence")
				emit signedPresenceReceived (from);
		}
		
		// Case 2: encrypted message
		if (x_element.namespaceURI () == NsEncrypted)
		{
			QString encryptedBodyStr = x_element.text ();
			//TODO Check if we need another representation, instead of 'toAscii()'
			const QByteArray& encryptedBody = encryptedBodyStr.toAscii ();
			QByteArray decryptedBody = DecryptBody (encryptedBody);
			if (!decryptedBody.isEmpty ())
				emit encryptedMessageReceived (from, QString::fromUtf8 (decryptedBody));
		}

		return false;
	}
}
}
}
