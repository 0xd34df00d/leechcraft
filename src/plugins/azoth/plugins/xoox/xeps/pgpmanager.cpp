/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pgpmanager.h"
#include <QDomElement>
#include <QtCrypto>
#include <QXmppClient.h>
#include <util/sll/unreachable.h>
#include <interfaces/azoth/gpgexceptions.h>

namespace LC
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
		enum class PGPType
		{
			Signature,
			Message
		};

		QString GetAsciiString (PGPType type)
		{
			switch (type)
			{
			case PGPType::Message:
				return "MESSAGE";
			case PGPType::Signature:
				return "SIGNATURE";
			}

			Util::Unreachable ();
		}

		QString WrapPGP (const QString& str, PGPType type)
		{
			const auto& typeStr = GetAsciiString (type);

			const auto& startMarker = QString { "-----BEGIN PGP %1-----\n" }.arg (typeStr);
			const auto& endMarker = QString { "-----END PGP %1-----\n" }.arg (typeStr);

			if (str.contains (startMarker) && str.contains (endMarker))
				return str;

			QString result;
			result += startMarker;
			result += "Version: PGP\n\n";
			result += str + "\n";
			result += endMarker;
			return result;
		}
	}

	QByteArray PgpManager::EncryptBody (const QCA::PGPKey& pubkey, const QByteArray& body)
	{
		if (pubkey.isNull ())
		{
			warning ("Cannot encrypt: public key is null");
			throw GPGExceptions::NullPubkey {};
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

		if (!msg.success ())
		{
			info (QString { "Error encrypting: %1 (%2)." }
						.arg (msg.errorCode ())
						.arg (msg.diagnosticText ()));
			throw GPGExceptions::Encryption { msg.errorCode (), msg.diagnosticText () };
		}

		return msg.read ();
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
			warning (QString { "Error signing: %1 (%2)." }
						.arg (msg.errorCode ())
						.arg (msg.diagnosticText ()));
			return QByteArray ();
		}

		const auto& sig = msg.signature ();
		const auto& arrs = sig.split ('\n');
		auto it = arrs.begin ();
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
			info (QString { "Error signing: %1" }
						.arg (msg.errorCode ())
						.arg (msg.diagnosticText ()));
			return QByteArray ();
		}
	}

	QByteArray PgpManager::DecryptBody (const QByteArray& encrypted)
	{
		QCA::OpenPGP pgp;
		QCA::SecureMessage msg (&pgp);
		msg.setFormat (QCA::SecureMessage::Ascii);
		msg.startDecrypt ();
		msg.update (WrapPGP (encrypted, PGPType::Message).toUtf8 ());
		msg.end ();
		msg.waitForFinished ();

		if (msg.success ())
			return msg.read ();
		else
		{
			info (QString ("Error decrypting: %1").arg (msg.errorCode ()));
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
		msg.startVerify (WrapPGP (signature, PGPType::Signature).toUtf8 ());
		msg.update (message);
		msg.end ();
		msg.waitForFinished ();

		if (msg.verifySuccess ())
			return true;
		else
		{
			info (QString ("Invalid signature: %1").arg (msg.errorCode ()));
			return false;
		}
	}

	bool PgpManager::handleStanza (const QDomElement& stanza)
	{
		const auto& tagName = stanza.tagName ();
		if ("message" != tagName && "presence" != tagName)
			return false;

		const auto& from = stanza.attribute ("from");

		// Case 1: signed presence|message
		const auto& x_element = stanza.firstChildElement ("x");
		if (x_element.namespaceURI () == NsSigned)
		{
			const auto& status = stanza.firstChildElement ("status");
			const auto& message = status.text ();
			const auto& signature = x_element.text ();

			const QCA::PGPKey key = PublicKey (from);

			if (!IsValidSignature (key, message.toUtf8 (), signature.toLatin1 ()))
				emit invalidSignatureReceived (from);
			else if (tagName == "message")
				emit signedMessageReceived (from);
			else if (tagName == "presence")
				emit signedPresenceReceived (from);
		}

		// Case 2: encrypted message
		if (x_element.namespaceURI () == NsEncrypted)
		{
			const auto& encryptedBodyStr = x_element.text ();
			const auto& encryptedBody = encryptedBodyStr.toLatin1 ();
			const auto& decryptedBody = DecryptBody (encryptedBody);
			if (!decryptedBody.isEmpty ())
				emit encryptedMessageReceived (from, QString::fromUtf8 (decryptedBody));
		}

		return false;
	}
}
}
}
