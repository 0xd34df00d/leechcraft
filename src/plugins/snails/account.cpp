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

#include "account.h"
#include <stdexcept>
#include <QUuid>
#include <QDataStream>
#include <QInputDialog>
#include <QUrl>
#include <QAuthenticator>
#include <util/util.h>
#include "proto/Imap/Model/Model.h"
#include "proto/Imap/Model/MemoryCache.h"
#include "proto/Imap/Model/MailboxModel.h"
#include "proto/Imap/Model/PrettyMailboxModel.h"
#include "proto/Imap/Model/MsgListModel.h"
#include "proto/Imap/Model/ThreadingMsgListModel.h"
#include "proto/Imap/Model/PrettyMsgListModel.h"
#include "proto/Streams/SocketFactory.h"
#include "core.h"
#include "accountconfigdialog.h"
#include "storage.h"
#include "accountfoldermanager.h"

namespace LeechCraft
{
namespace Snails
{
	Account::Account (QObject *parent)
	: QObject (parent)
	, ID_ (QUuid::createUuid ().toByteArray ())
	, UseSASL_ (false)
	, SASLRequired_ (false)
	, UseTLS_ (true)
	, TLSRequired_ (false)
	, SMTPNeedsAuth_ (true)
	, APOP_ (false)
	, APOPFail_ (false)
	, FolderManager_ (new AccountFolderManager (this))
	, Model_ (0)
	, PrettyMboxModel_ (0)
	, MsgListModel_ (0)
	, PrettyMsgListModel_ (0)
	{
	}

	QByteArray Account::GetID () const
	{
		return ID_;
	}

	QString Account::GetName () const
	{
		return AccName_;
	}

	QString Account::GetServer () const
	{
		return InType_ == InType::Maildir ?
			QString () :
			InHost_ + ':' + QString::number (InPort_);
	}

	QString Account::GetType () const
	{
		switch (InType_)
		{
		case InType::IMAP:
			return "IMAP";
		case InType::POP3:
			return "POP3";
		case InType::Maildir:
			return "Maildir";
		default:
			return "<unknown>";
		}
	}

	AccountFolderManager* Account::GetFolderManager () const
	{
		return FolderManager_;
	}

	QAbstractItemModel* Account::GetFoldersModel () const
	{
		return PrettyMboxModel_;
	}

	QAbstractItemModel* Account::GetItemsModel () const
	{
		return PrettyMsgListModel_;
	}

	void Account::Synchronize (Account::FetchFlags flags)
	{
		qDebug () << Q_FUNC_INFO;
		Model_->reloadMailboxList ();
	}

	void Account::FetchWholeMessage (Message_ptr msg)
	{
	}

	void Account::SendMessage (Message_ptr msg)
	{
	}

	void Account::FetchAttachment (Message_ptr msg,
			const QString& attName, const QString& path)
	{
	}

	QByteArray Account::Serialize () const
	{
		QByteArray result;

		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (3);
		out << ID_
			<< AccName_
			<< Login_
			<< UseSASL_
			<< SASLRequired_
			<< UseTLS_
			<< TLSRequired_
			<< SMTPNeedsAuth_
			<< APOP_
			<< APOPFail_
			<< InHost_
			<< InPort_
			<< OutHost_
			<< OutPort_
			<< OutLogin_
			<< static_cast<quint8> (InType_)
			<< static_cast<quint8> (OutType_)
			<< UserName_
			<< UserEmail_
			<< FolderManager_->Serialize ();

		return result;
	}

	void Account::Deserialize (const QByteArray& arr)
	{
		QDataStream in (arr);
		quint8 version = 0;
		in >> version;

		if (version < 1 || version > 3)
			throw std::runtime_error (qPrintable ("Unknown version " + QString::number (version)));

		quint8 inType = 0, outType = 0;

		in >> ID_
			>> AccName_
			>> Login_
			>> UseSASL_
			>> SASLRequired_
			>> UseTLS_
			>> TLSRequired_
			>> SMTPNeedsAuth_
			>> APOP_
			>> APOPFail_
			>> InHost_
			>> InPort_
			>> OutHost_
			>> OutPort_
			>> OutLogin_
			>> inType
			>> outType;

		InType_ = static_cast<InType> (inType);
		OutType_ = static_cast<OutType> (outType);

		if (version >= 2)
			in >> UserName_
				>> UserEmail_;

		if (version >= 3)
		{
			QByteArray fstate;
			in >> fstate;
			FolderManager_->Deserialize (fstate);
		}

		ReinitModel ();
	}

	void Account::OpenConfigDialog ()
	{
		std::unique_ptr<AccountConfigDialog> dia (new AccountConfigDialog);

		dia->SetName (AccName_);
		dia->SetUserName (UserName_);
		dia->SetUserEmail (UserEmail_);
		dia->SetLogin (Login_);
		dia->SetUseSASL (UseSASL_);
		dia->SetSASLRequired (SASLRequired_);
		dia->SetUseTLS (UseTLS_);
		dia->SetTLSRequired (TLSRequired_);
		dia->SetSMTPAuth (SMTPNeedsAuth_);
		dia->SetAPOP (APOP_);
		dia->SetAPOPRequired (APOPFail_);
		dia->SetInHost (InHost_);
		dia->SetInPort (InPort_);
		dia->SetOutHost (OutHost_);
		dia->SetOutPort (OutPort_);
		dia->SetOutLogin (OutLogin_);
		dia->SetInType (InType_);
		dia->SetOutType (OutType_);

		const auto& folders = FolderManager_->GetFolders ();
		dia->SetAllFolders (folders);
		const auto& toSync = FolderManager_->GetSyncFolders ();
		Q_FOREACH (const auto& folder, folders)
		{
			const auto flags = FolderManager_->GetFolderFlags (folder);
			if (flags & AccountFolderManager::FolderOutgoing)
				dia->SetOutFolder (folder);
		}
		dia->SetFoldersToSync (toSync);

		if (dia->exec () != QDialog::Accepted)
			return;

		AccName_ = dia->GetName ();
		UserName_ = dia->GetUserName ();
		UserEmail_ = dia->GetUserEmail ();
		Login_ = dia->GetLogin ();
		UseSASL_ = dia->GetUseSASL ();
		SASLRequired_ = dia->GetSASLRequired ();
		UseTLS_ = dia->GetUseTLS ();
		TLSRequired_ = dia->GetTLSRequired ();
		SMTPNeedsAuth_ = dia->GetSMTPAuth ();
		APOP_ = dia->GetAPOP ();
		APOPFail_ = dia->GetAPOPRequired ();
		InHost_ = dia->GetInHost ();
		InPort_ = dia->GetInPort ();
		OutHost_ = dia->GetOutHost ();
		OutPort_ = dia->GetOutPort ();
		OutLogin_ = dia->GetOutLogin ();
		InType_ = dia->GetInType ();
		OutType_ = dia->GetOutType ();

		FolderManager_->ClearFolderFlags ();
		const auto& out = dia->GetOutFolder ();
		if (!out.isEmpty ())
			FolderManager_->AppendFolderFlags (out, AccountFolderManager::FolderOutgoing);

		Q_FOREACH (const auto& sync, dia->GetFoldersToSync ())
			FolderManager_->AppendFolderFlags (sync, AccountFolderManager::FolderSyncable);

		ReinitModel ();

		emit accountChanged ();
	}

	bool Account::IsNull () const
	{
		return AccName_.isEmpty () ||
			Login_.isEmpty ();
	}

	QString Account::GetInUsername ()
	{
		return Login_;
	}

	QString Account::GetOutUsername ()
	{
		return OutLogin_;
	}

	void Account::handleFolderActivated (const QModelIndex& index)
	{
		MsgListModel_->setMailbox (index);
	}

	QString Account::GetPassImpl (Direction dir)
	{
		QList<QVariant> keys;
		keys << GetStoreID (dir);
		const QVariantList& result =
			Util::GetPersistentData (keys, &Core::Instance ());
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}

	QByteArray Account::GetStoreID (Account::Direction dir) const
	{
		QByteArray result = GetID ();
		if (dir == Direction::Out)
			result += "/out";
		return result;
	}

	void Account::ReinitModel ()
	{
		if (Model_)
			Model_->deleteLater ();

		Imap::Mailbox::SocketFactoryPtr factory;
		if (UseTLS_)
			factory.reset (new Imap::Mailbox::TlsAbleSocketFactory (InHost_, InPort_));
		else
			factory.reset (new Imap::Mailbox::SslSocketFactory (InHost_, InPort_));

		Model_ = new Imap::Mailbox::Model (this,
				new Imap::Mailbox::MemoryCache (this, QString ()),
				factory,
				Imap::Mailbox::TaskFactoryPtr (new Imap::Mailbox::TaskFactory ()),
				false);
		Model_->setObjectName ("model");
		auto mboxModel = new Imap::Mailbox::MailboxModel (Model_, Model_);
		PrettyMboxModel_ = new Imap::Mailbox::PrettyMailboxModel (Model_, mboxModel);

		connect (Model_,
				SIGNAL (authRequested (QAuthenticator*)),
				this,
				SLOT (handleAuthRequested (QAuthenticator*)));
		connect (Model_,
				SIGNAL (logged (uint, Imap::Mailbox::LogMessage)),
				this,
				SLOT (handleLogged (uint, Imap::Mailbox::LogMessage)));

		MsgListModel_ = new Imap::Mailbox::MsgListModel (Model_, Model_);
		PrettyMsgListModel_ = new Imap::Mailbox::PrettyMsgListModel (Model_);
		PrettyMsgListModel_->setSourceModel (MsgListModel_);
	}

	void Account::handleAuthRequested (QAuthenticator *auth)
	{
		auth->setUser (Login_);
		auth->setPassword (GetPassImpl (Direction::In));
	}

	void Account::handleLogged (uint code, Imap::Mailbox::LogMessage msg)
	{
		qDebug () << "[IMAP]" << GetName () << code << msg.kind << msg.message << msg.source;
	}

	// TODO migrate to stuff from Util.
	void Account::getPassword (QString *outPass, Direction dir)
	{
		QString pass = GetPassImpl (dir);
		if (!pass.isEmpty ())
		{
			*outPass = pass;
			return;
		}

		pass = QInputDialog::getText (0,
				"LeechCraft",
				Account::tr ("Enter password for account %1:")
						.arg (GetName ()),
				QLineEdit::Password);
		*outPass = pass;
		if (pass.isEmpty ())
			return;

		QList<QVariant> keys;
		keys << GetStoreID (dir);

		QList<QVariant> passwordVar;
		passwordVar << pass;
		QList<QVariant> values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		Core::Instance ().SendEntity (e);
	}

	void Account::handleMsgHeaders (QList<Message_ptr> messages)
	{
		Core::Instance ().GetStorage ()->SaveMessages (this, messages);
		emit mailChanged ();
		emit gotNewMessages (messages);
	}

	void Account::handleGotUpdatedMessages (QList<Message_ptr> messages)
	{
		Core::Instance ().GetStorage ()->SaveMessages (this, messages);
		emit mailChanged ();
		emit gotNewMessages (messages);
	}

	void Account::handleGotFolders (QList<QStringList> folders)
	{
		FolderManager_->SetFolders (folders);
	}

	void Account::handleMessageBodyFetched (Message_ptr msg)
	{
		Core::Instance ().GetStorage ()->SaveMessages (this, { msg });
		emit messageBodyFetched (msg);
	}
}
}
