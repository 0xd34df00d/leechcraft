/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "crypthandler.h"
#include <QtDebug>

#ifdef ENABLE_CRYPT
#include "xeps/pgpmanager.h"
#endif

#include "clientconnection.h"
#include "glooxmessage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	CryptHandler::CryptHandler (ClientConnection *conn)
	: QObject (conn)
	, Conn_ (conn)
	{
	}

	void CryptHandler::Init ()
	{
#ifdef ENABLE_CRYPT
		PGPManager_ = new PgpManager ();

		Conn_->GetClient ()->addExtension (PGPManager_);
		connect (PGPManager_,
				SIGNAL (encryptedMessageReceived (QString, QString)),
				this,
				SLOT (handleEncryptedMessageReceived (QString, QString)));
		connect (PGPManager_,
				SIGNAL (signedMessageReceived (const QString&)),
				this,
				SLOT (handleSignedMessageReceived (const QString&)));
		connect (PGPManager_,
				SIGNAL (signedPresenceReceived (const QString&)),
				this,
				SLOT (handleSignedPresenceReceived (const QString&)));
		connect (PGPManager_,
				SIGNAL (invalidSignatureReceived (const QString&)),
				this,
				SLOT (handleInvalidSignatureReceived (const QString&)));
#endif
	}

	void CryptHandler::HandlePresence (const QXmppPresence&, const QString& jid, const QString&)
	{
		if (SignedPresences_.remove (jid))
		{
			qDebug () << "got signed presence" << jid;
		}
	}

	void CryptHandler::ProcessOutgoing (QXmppMessage& msg, GlooxMessage *msgObj)
	{
#ifdef ENABLE_CRYPT
		EntryBase *entry = qobject_cast<EntryBase*> (msgObj->OtherPart ());
		if (!entry || !Entries2Crypt_.contains (entry->GetJID ()))
			return;

		const auto& key = PGPManager_->PublicKey (entry->GetJID ());

		if (key.isNull ())
			return;

		const auto& body = msg.body ();

		QXmppElement crypt;
		crypt.setTagName ("x");
		crypt.setAttribute ("xmlns", "jabber:x:encrypted");
		crypt.setValue (PGPManager_->EncryptBody (key, body.toUtf8 ()));

		msg.setExtensions (msg.extensions () << crypt);

		msg.setBody (tr ("This message is encrypted. Please decrypt "
						"it to view the original contents."));
#endif
	}

	void CryptHandler::ProcessIncoming (QXmppMessage& msg)
	{
		if (EncryptedMessages_.contains (msg.from ()))
			msg.setBody (EncryptedMessages_.take (msg.from ()));
	}

#ifdef ENABLE_CRYPT
	PgpManager* CryptHandler::GetPGPManager () const
	{
		return PGPManager_;
	}

	bool CryptHandler::SetEncryptionEnabled (const QString& jid, bool enabled)
	{
		if (enabled)
			Entries2Crypt_ << jid;
		else
			Entries2Crypt_.remove (jid);

		return true;
	}

	bool CryptHandler::IsEncryptionEnabled (const QString& jid) const
	{
		return Entries2Crypt_.contains (jid);
	}
#endif

	void CryptHandler::handleEncryptedMessageReceived (const QString& id, const QString& decrypted)
	{
		EncryptedMessages_ [id] = decrypted;
	}

	void CryptHandler::handleSignedMessageReceived (const QString&)
	{
	}

	void CryptHandler::handleSignedPresenceReceived (const QString& id)
	{
		SignedPresences_ << id;
	}

	void CryptHandler::handleInvalidSignatureReceived (const QString& id)
	{
		qDebug () << Q_FUNC_INFO << id;
	}
}
}
}
