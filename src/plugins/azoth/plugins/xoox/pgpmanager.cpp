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
	QCA::PGPKey QXmppPgpManager::PublicKey (const QString& id) const
	{
		return ((PublicKeys_.contains (id)) ? PublicKeys_[id] : QString ());
	}

	void QXmppPgpManager::SetPublicKey (const QString& id, const QCA::PGPKey& publicKey)
	{
		PublicKeys_.insert (id, publicKey);
	}

	QCA::PGPKey QXmppPgpManager::PrivateKey () const
	{
		return PrivateKey_;
	}

	void QXmppPgpManager::SetPrivateKey (const QCA::PGPKey& privateKey)
	{
		PrivateKey_ = privateKey;
	}

	bool QXmppPgpManager::EncryptBody(const QCA::PGPKey& pubkey, const QByteArray& body, QByteArray& encrypted)
	{
		QCA::SecureMessageKey msgKey;
		if (pubkey.isNull ())
		{
			warning (QString ("Cannot encrypt: public key is null"));
			return false;
		}
		msgKey.setPGPPublicKey (pubkey);
		const QByteArray originalText = body;
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setRecipient (msgKey);
		msg.startEncrypt ();
		msg.update (originalText);
		msg.end ();
		msg.waitForFinished (-1);
		if (msg.success ())
		{
			encrypted = msg.read ();
			return true;
		}
		else
		{
			info (QString ("Error encrypting: " + msg.errorCode ()));
			return false;
		}
	}

	bool QXmppPgpManager::SignMessage (const QByteArray& body, QByteArray& signature)
	{
		QCA::SecureMessageKey msgKey;
		if (PrivateKey_.isNull ())
		{
			warning (QString ("Cannot sign: private key is null"));
			return false;
		}
		msgKey.setPGPSecretKey (PrivateKey_);
		const QByteArray originalText = body;
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.setSigner (msgKey);
		msg.startSign (QCA::SecureMessage::Detached);
		msg.update (originalText);
		msg.end ();
		msg.waitForFinished (-1);
		if (msg.success ())
		{
			signature = msg.signature ();
			return true;
		}
		else
		{
			warning (QString ("Error signing: " + msg.errorCode ()));
			return false;
		}
	}

	bool QXmppPgpManager::SignPresence (const QByteArray& status, QByteArray& signature)
	{
		QCA::SecureMessageKey msgKey;
		if (PrivateKey_.isNull ())
		{
			warning (QString ("Cannot sign: private key is null"));
			return false;
		}
		msgKey.setPGPSecretKey (PrivateKey_);
		const QByteArray originalText = status;
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.setSigner (msgKey);
		msg.startSign (QCA::SecureMessage::Detached);
		msg.update (originalText);
		msg.end ();
		msg.waitForFinished (-1);
		if (msg.success ())
		{
			signature = msg.signature ();
			return true;
		}
		else
		{
			info (QString ("Error signing: " + msg.errorCode ()));
			return false;
		}
	}

	bool QXmppPgpManager::DecryptBody (const QByteArray& encrypted, QByteArray& decrypted)
	{
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.startDecrypt ();
		msg.update (encrypted);
		msg.end ();
		msg.waitForFinished (-1);
		if (msg.success ())
		{
			decrypted = msg.read ();
			return true;
		}
		else
		{
			info (QString ("Error decrypting: " + msg.errorCode ()));
			return false;
		}
	}

	bool QXmppPgpManager::IsValidSignature (const QCA::PGPKey& pubkey, const QByteArray& message, const QByteArray& signature)
	{
		QCA::OpenPGP pgp;
		QCA::SecureMessageKey key;
		QCA::SecureMessage msg (&pgp);
		key.setPGPPublicKey (pubkey);
		msg.setSigner (key);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.startVerify (signature);
		msg.update (message);
		msg.end ();
		msg.waitForFinished (-1);
		if (msg.verifySuccess ())
			return true;
		else
		{
			info(QString("Invalid signature: " + msg.errorCode ()));
			return false;
		}
	}

	bool QXmppPgpManager::handleStanza (const QDomElement &stanza)
	{
		if ((stanza.tagName () != "message") || (stanza.tagName () != "presence"))
			return false;

		QString from = stanza.attribute ("from");
		QString to = stanza.attribute ("to");

		// Case 1: signed presence|message
		const QDomElement &x_element = stanza.firstChildElement ("x");
		if (x_element.namespaceURI () == ns_signed)
		{
			const QDomElement &status = stanza.firstChildElement ("status");
			QString message = status.text ();
			QString signature = x_element.text ();
			//TODO Initialize keystore somewhere
			//TODO Check if we need another representation, instead of 'toAscii()'
			if (!IsValidSignature (PublicKey ("from"), message.toAscii (), signature.toAscii ()))
			{
				emit invalidSignatureReceived (stanza);
				return false;
			}
			else
			{
				if (stanza.tagName () == "message")
				{
					QXmppMessage msg (from, to);
					QString body = stanza.firstChildElement ("body").text ();
					msg.setBody (body);
					emit signedMessageReceived (msg);
					return true;
				}
				if (stanza.tagName () == "presence")
				{
					QString typeText = stanza.attribute ("type");
					QString statusText = stanza.firstChildElement ("status").text ();
					QXmppPresence::Status status;
					status.setStatusText (statusText);
					QXmppPresence::Type type = SetPresenceTypeFromStr (typeText);
					QXmppPresence presence (type, status);
					emit signedPresenceReceived (presence);
					return true;
				}
			}
		}
		// Case 2: encrypted message
		if (x_element.namespaceURI () == ns_encrypted)
		{
			QString encryptedBodyStr = x_element.text ();
			QByteArray encryptedBody = encryptedBodyStr.toAscii ();
			QByteArray decryptedBody ("");
			//TODO Check if we need another representation, instead of 'toAscii()'
			bool decryptSuccess = DecryptBody (encryptedBody, decryptedBody);
			if (decryptSuccess)
			{
				QXmppMessage msg (from, to);
				msg.setBody (QString (decryptedBody));
				emit encryptedMessageReceived (msg);
				return true;
			}
		}
		return false;
	}


	QXmppPresence::Type QXmppPgpManager::SetPresenceTypeFromStr (const QString& str)
	{
		QXmppPresence::Type type;
		if (str == "error")
			type = QXmppPresence::Error;
		else if (str == "unavailable")
			type = QXmppPresence::Unavailable;
		else if (str == "subscribe")
			type = QXmppPresence::Subscribe;
		else if (str == "subscribed")
			type = QXmppPresence::Subscribed;
		else if (str == "unsubscribe")
			type = QXmppPresence::Unsubscribe;
		else if (str == "unsubscribed")
			type = QXmppPresence::Unsubscribed;
		else if (str == "probe")
			type = QXmppPresence::Probe;
		else if (str == "")
			type = QXmppPresence::Available;
		else
		{
			type = static_cast<QXmppPresence::Type> (-1);
			qWarning("QXmppPgpManager::setPresenceTypeFromStr () invalid input string type: %s",
				 qPrintable (str));
		}
		return type;
	}
}
}
}
