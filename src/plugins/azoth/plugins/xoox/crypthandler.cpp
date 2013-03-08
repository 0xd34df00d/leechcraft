/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "crypthandler.h"
#include <QtDebug>

#ifdef ENABLE_CRYPT
#include "pgpmanager.h"
#endif

#include "clientconnection.h"
#include "glooxmessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	CryptHandler::CryptHandler (ClientConnection *conn)
	: QObject (conn)
	, Conn_ (conn)
#ifdef ENABLE_CRYPT
	, PGPManager_ (0)
#endif
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

		const QCA::PGPKey& key = PGPManager_->PublicKey (entry->GetJID ());

		if (!key.isNull ())
		{
			const QString& body = msg.body ();
			msg.setBody (tr ("This message is encrypted. Please decrypt "
							"it to view the original contents."));

			QXmppElement crypt;
			crypt.setTagName ("x");
			crypt.setAttribute ("xmlns", "jabber:x:encrypted");
			crypt.setValue (PGPManager_->EncryptBody (key, body.toUtf8 ()));

			msg.setExtensions (msg.extensions () << crypt);
		}
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
#endif

	void CryptHandler::handleEncryptedMessageReceived (const QString& id,
			const QString& decrypted)
	{
		EncryptedMessages_ [id] = decrypted;
	}

	void CryptHandler::handleSignedMessageReceived (const QString&)
	{
	}

	void CryptHandler::handleSignedPresenceReceived (const QString&)
	{
	}

	void CryptHandler::handleInvalidSignatureReceived (const QString& id)
	{
		qDebug () << Q_FUNC_INFO << id;
	}
}
}
}
