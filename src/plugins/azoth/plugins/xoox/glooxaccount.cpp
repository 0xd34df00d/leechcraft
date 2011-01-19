/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "glooxaccount.h"
#include <memory>
#include <QInputDialog>
#include <QtDebug>
#include <interfaces/iprotocol.h>
#include <interfaces/iproxyobject.h>
#include "glooxprotocol.h"
#include "glooxaccountconfigurationdialog.h"
#include "core.h"
#include "clientconnection.h"
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "roomclentry.h"
#include "unauthclentry.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	GlooxAccount::GlooxAccount (const QString& name,
			QObject *parent)
	: QObject (parent)
	, Name_ (name)
	, ParentProtocol_ (qobject_cast<GlooxProtocol*> (parent))
	{
		AccState_.State_ = SOffline;
		AccState_.Priority_ = -1;

		connect (this,
				SIGNAL (scheduleClientDestruction ()),
				this,
				SLOT (handleDestroyClient ()),
				Qt::QueuedConnection);
	}

	void GlooxAccount::Init ()
	{
		ClientConnection_.reset (new ClientConnection (JID_ + "/" + Resource_,
						AccState_, this));
		connect (ClientConnection_.get (),
				SIGNAL (serverAuthFailed ()),
				this,
				SLOT (handleServerAuthFailed ()));
		connect (ClientConnection_.get (),
				SIGNAL (needPassword ()),
				this,
				SLOT (feedClientPassword ()));

		connect (ClientConnection_.get (),
				SIGNAL (gotRosterItems (const QList<QObject*>&)),
				this,
				SLOT (handleGotRosterItems (const QList<QObject*>&)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemRemoved (QObject*)),
				this,
				SLOT (handleEntryRemoved (QObject*)));
		connect (ClientConnection_.get (),
				SIGNAL (rosterItemsRemoved (const QList<QObject*>&)),
				this,
				SIGNAL (removedCLItems (const QList<QObject*>&)));
		connect (ClientConnection_.get (),
				SIGNAL (gotSubscriptionRequest (QObject*, const QString&)),
				this,
				SIGNAL (authorizationRequested (QObject*, const QString&)));
	}

	QObject* GlooxAccount::GetObject ()
	{
		return this;
	}

	QObject* GlooxAccount::GetParentProtocol () const
	{
		return ParentProtocol_;
	}

	IAccount::AccountFeatures GlooxAccount::GetAccountFeatures () const
	{
		return FRenamable | FSupportsXA;
	}

	QList<QObject*> GlooxAccount::GetCLEntries ()
	{
		return ClientConnection_ ?
				ClientConnection_->GetCLEntries () :
				QList<QObject*> ();
	}

	QString GlooxAccount::GetAccountName () const
	{
		return Name_;
	}

	QString GlooxAccount::GetOurNick () const
	{
		return Nick_;
	}

	void GlooxAccount::RenameAccount (const QString& name)
	{
		Name_ = name;
	}

	QByteArray GlooxAccount::GetAccountID () const
	{
		return ParentProtocol_->GetProtocolID () + "_" + JID_.toUtf8 ();
	}

	void GlooxAccount::QueryInfo (const QString& entryId)
	{
		ClientConnection_->FetchVCard (entryId);
	}

	void GlooxAccount::OpenConfigurationDialog ()
	{
		// TODO nonmodal
		std::auto_ptr<GlooxAccountConfigurationDialog> dia (new GlooxAccountConfigurationDialog (0));
		if (!JID_.isEmpty ())
			dia->SetJID (JID_);
		if (!Nick_.isEmpty ())
			dia->SetNick (Nick_);
		if (!Resource_.isEmpty ())
			dia->SetResource (Resource_);
		dia->SetPriority (AccState_.Priority_);
		if (dia->exec () == QDialog::Rejected)
			return;

		JID_ = dia->GetJID ();
		Nick_ = dia->GetNick ();
		Resource_ = dia->GetResource ();
		AccState_.Priority_ = dia->GetPriority ();

		emit accountSettingsChanged ();

		ChangeState (EntryStatus (AccState_.State_, AccState_.Status_));
	}

	EntryStatus GlooxAccount::GetState () const
	{
		return EntryStatus (AccState_.State_, AccState_.Status_);
	}

	void GlooxAccount::ChangeState (const EntryStatus& status)
	{
		if (status.State_ == SOffline &&
				!ClientConnection_)
			return;

		AccState_.State_ = status.State_;
		AccState_.Status_ = status.StatusString_;

		if (!ClientConnection_)
			Init ();

		ClientConnection_->SetState (AccState_);

		emit statusChanged (status);
	}

	void GlooxAccount::Synchronize ()
	{
		ClientConnection_->Synchronize ();
	}

	void GlooxAccount::Authorize (QObject *entryObj)
	{
		ClientConnection_->AckAuth (entryObj, true);
	}

	void GlooxAccount::DenyAuth (QObject *entryObj)
	{
		ClientConnection_->AckAuth (entryObj, false);
	}

	void GlooxAccount::RequestAuth (const QString& entryId,
			const QString& msg, const QString& name, const QStringList& groups)
	{
		ClientConnection_->Subscribe (entryId, msg, name, groups);
	}

	void GlooxAccount::RemoveEntry (QObject *entryObj)
	{
		GlooxCLEntry *entry = qobject_cast<GlooxCLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is not a GlooxCLEntry";
			return;
		}

		ClientConnection_->Remove (entry);
	}

	QString GlooxAccount::GetJID () const
	{
		return JID_;
	}

	QString GlooxAccount::GetNick () const
	{
		return Nick_;
	}

	void GlooxAccount::JoinRoom (const QString& server,
			const QString& room, const QString& nick)
	{
		QString jidStr = QString ("%1@%2")
				.arg (room, server);

		RoomCLEntry *entry = ClientConnection_->JoinRoom (jidStr, nick);
		if (!entry)
			return;
		emit gotCLItems (QList<QObject*> () << entry);
	}

	boost::shared_ptr<ClientConnection> GlooxAccount::GetClientConnection () const
	{
		return ClientConnection_;
	}

	GlooxCLEntry* GlooxAccount::CreateFromODS (GlooxCLEntry::OfflineDataSource_ptr ods)
	{
		return ClientConnection_->AddODSCLEntry (ods);
	}

	QByteArray GlooxAccount::Serialize () const
	{
		quint16 version = 1;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
				<< Name_
				<< JID_
				<< Nick_
				<< Resource_
				<< AccState_.Priority_;
		}

		return result;
	}

	GlooxAccount* GlooxAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString name;
		in >> name;
		GlooxAccount *result = new GlooxAccount (name, parent);
		in >> result->JID_
			>> result->Nick_
			>> result->Resource_
			>> result->AccState_.Priority_;
		result->Init ();

		return result;
	}

	QObject* GlooxAccount::CreateMessage (IMessage::MessageType type,
			const QString& variant,
			const QString& body,
			const QXmppRosterIq::Item& ri)
	{
		return ClientConnection_->CreateMessage (type, variant, body, ri);
	}

	QString GlooxAccount::GetPassword (bool authfailure)
	{
		IProxyObject *proxy =
			qobject_cast<IProxyObject*> (ParentProtocol_->GetProxyObject ());
		if (!authfailure)
		{
			const QString& result = proxy->GetPassword (this);
			if (!result.isNull ())
				return result;
		}

		QString result = QInputDialog::getText (0,
				"LeechCraft",
				tr ("Enter password for %1:").arg (JID_), QLineEdit::Password);
		if (!result.isNull ())
			proxy->SetPassword (result, this);
		return result;
	}

	void GlooxAccount::handleEntryRemoved (QObject *entry)
	{
		emit removedCLItems (QObjectList () << entry);
	}

	void GlooxAccount::handleGotRosterItems (const QList<QObject*>& items)
	{
		emit gotCLItems (items);
	}

	void GlooxAccount::handleServerAuthFailed ()
	{
		const QString& pwd = GetPassword (true);
		if (!pwd.isNull ())
		{
			ClientConnection_->SetPassword (pwd);
			ChangeState (EntryStatus (AccState_.State_, AccState_.Status_));
		}
	}

	void GlooxAccount::feedClientPassword ()
	{
		ClientConnection_->SetPassword (GetPassword ());
	}

	void GlooxAccount::handleDestroyClient ()
	{
		ClientConnection_.reset ();
	}
}
}
}
}
}
