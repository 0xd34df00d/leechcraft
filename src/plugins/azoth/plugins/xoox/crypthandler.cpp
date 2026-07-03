/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "crypthandler.h"
#include <QtDebug>
#include <QXmppXmlElement.h>

#ifdef ENABLE_CRYPT
#include "xeps/pgpmanager.h"
#endif

#include "clientconnection.h"
#include "glooxmessage.h"

namespace LC::Azoth::Xoox
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
				&PgpManager::encryptedMessageReceived,
				this,
				[this] (const QString& id, const QString& decrypted) { EncryptedMessages_ [id] = decrypted; });
		connect (PGPManager_,
				&PgpManager::signedMessageReceived,
				this,
				[] {});
		connect (PGPManager_,
				&PgpManager::signedPresenceReceived,
				this,
				[this] (const QString& id) { SignedPresences_ << id; });
		connect (PGPManager_,
				&PgpManager::invalidSignatureReceived,
				this,
				[] (const QString& id) { qWarning () << "invalid PGP signature from" << id; });
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

		QXmpp::Xml::Element crypt { "x", "jabber:x:encrypted" };
		crypt.setText (PGPManager_->EncryptBody (key, body.toUtf8 ()));
		msg.extensions ().add (std::move (crypt));

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
}
