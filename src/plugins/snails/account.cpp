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
#include <QMutex>
#include <QStandardItemModel>
#include <util/util.h>
#include "core.h"
#include "accountconfigdialog.h"
#include "accountthread.h"
#include "accountthreadworker.h"
#include "storage.h"
#include "accountfoldermanager.h"
#include "mailmodelmanager.h"

namespace LeechCraft
{
namespace Snails
{
	Account::Account (QObject *parent)
	: QObject (parent)
	, Thread_ (new AccountThread (this))
	, AccMutex_ (new QMutex (QMutex::Recursive))
	, ID_ (QUuid::createUuid ().toByteArray ())
	, UseSASL_ (false)
	, SASLRequired_ (false)
	, UseTLS_ (true)
	, UseSSL_ (false)
	, InSecurityRequired_ (false)
	, OutSecurity_ (SecurityType::SSL)
	, OutSecurityRequired_ (false)
	, SMTPNeedsAuth_ (true)
	, APOP_ (false)
	, APOPFail_ (false)
	, FolderManager_ (new AccountFolderManager (this))
	, FoldersModel_ (new QStandardItemModel (this))
	, MailModelMgr_ (new MailModelManager (this))
	{
		Thread_->start (QThread::LowPriority);
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
		QMutexLocker l (GetMutex ());
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

	QAbstractItemModel* Account::GetMailModel () const
	{
		return MailModelMgr_->GetModel ();
	}

	QAbstractItemModel* Account::GetFoldersModel () const
	{
		return FoldersModel_;
	}

	void Account::ShowFolder (const QModelIndex& idx)
	{
		MailModelMgr_->clear ();

		const QStringList& path = idx.data (FoldersRole::Path).toStringList ();
		if (path.isEmpty ())
			return;

		MailModelMgr_->SetCurrentFolder (path);

		QList<Message_ptr> messages;
		const auto& ids = Core::Instance ().GetStorage ()->LoadIDs (this, path);
		Q_FOREACH (const auto& id, ids)
			messages << Core::Instance ().GetStorage ()->LoadMessage (this, id);

		MailModelMgr_->appendMessages (messages);

		Synchronize (path);
	}

	void Account::Synchronize (Account::FetchFlags flags)
	{
		MailModelMgr_->clear ();
		MailModelMgr_->SetCurrentFolder (QStringList ("INBOX"));

		auto folders = FolderManager_->GetSyncFolders ();
		if (folders.isEmpty ())
			folders << QStringList ("INBOX");
		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"synchronize",
				Qt::QueuedConnection,
				Q_ARG (Account::FetchFlags, flags),
				Q_ARG (QList<QStringList>, folders));
	}

	void Account::Synchronize (const QStringList& path)
	{
		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"synchronize",
				Qt::QueuedConnection,
				Q_ARG (Account::FetchFlags, FetchFlag::FetchAll),
				Q_ARG (QList<QStringList>, { path }));
	}

	void Account::FetchWholeMessage (Message_ptr msg)
	{
		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"fetchWholeMessage",
				Qt::QueuedConnection,
				Q_ARG (Message_ptr, msg));
	}

	void Account::SendMessage (Message_ptr msg)
	{
		auto pair = msg->GetAddress (Message::Address::From);
		if (pair.first.isEmpty ())
			pair.first = UserName_;
		if (pair.second.isEmpty ())
			pair.second = UserEmail_;
		msg->SetAddress (Message::Address::From, pair);

		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"sendMessage",
				Qt::QueuedConnection,
				Q_ARG (Message_ptr, msg));
	}

	void Account::FetchAttachment (Message_ptr msg,
			const QString& attName, const QString& path)
	{
		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"fetchAttachment",
				Qt::QueuedConnection,
				Q_ARG (Message_ptr, msg),
				Q_ARG (QString, attName),
				Q_ARG (QString, path));
	}

	void Account::UpdateReadStatus (const QByteArray& id, bool isRead)
	{
		MailModelMgr_->UpdateReadStatus (id, isRead);
	}

	QByteArray Account::Serialize () const
	{
		QMutexLocker l (GetMutex ());

		QByteArray result;

		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (5);
		out << ID_
			<< AccName_
			<< Login_
			<< UseSASL_
			<< SASLRequired_
			<< UseTLS_
			<< UseSSL_
			<< InSecurityRequired_
			<< static_cast<qint8> (OutSecurity_)
			<< OutSecurityRequired_
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

		if (version < 1 || version > 5)
			throw std::runtime_error (qPrintable ("Unknown version " + QString::number (version)));

		quint8 inType = 0, outType = 0;

		{
			QMutexLocker l (GetMutex ());
			in >> ID_
				>> AccName_
				>> Login_
				>> UseSASL_
				>> SASLRequired_
				>> UseTLS_;

			if (version >= 4)
				in >> UseSSL_;
			else
				UseSSL_ = !UseTLS_;

			in >> InSecurityRequired_;

			if (version >= 5)
			{
				qint8 type = 0;
				in >> type
					>> OutSecurityRequired_;
				OutSecurity_ = static_cast<SecurityType> (type);
			}

			in >> SMTPNeedsAuth_
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
		}
	}

	void Account::OpenConfigDialog ()
	{
		std::unique_ptr<AccountConfigDialog> dia (new AccountConfigDialog);

		{
			QMutexLocker l (GetMutex ());
			dia->SetName (AccName_);
			dia->SetUserName (UserName_);
			dia->SetUserEmail (UserEmail_);
			dia->SetLogin (Login_);
			dia->SetUseSASL (UseSASL_);
			dia->SetSASLRequired (SASLRequired_);

			if (UseSSL_)
				dia->SetInSecurity (SecurityType::SSL);
			else if (UseTLS_)
				dia->SetInSecurity (SecurityType::TLS);
			else
				dia->SetInSecurity (SecurityType::No);

			dia->SetInSecurityRequired (InSecurityRequired_);

			dia->SetOutSecurity (OutSecurity_);
			dia->SetOutSecurityRequired (OutSecurityRequired_);

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
		}

		if (dia->exec () != QDialog::Accepted)
			return;

		{
			QMutexLocker l (GetMutex ());
			AccName_ = dia->GetName ();
			UserName_ = dia->GetUserName ();
			UserEmail_ = dia->GetUserEmail ();
			Login_ = dia->GetLogin ();
			UseSASL_ = dia->GetUseSASL ();
			SASLRequired_ = dia->GetSASLRequired ();

			UseSSL_ = false;
			UseTLS_ = false;
			switch (dia->GetInSecurity ())
			{
			case SecurityType::SSL:
				UseSSL_ = true;
				break;
			case SecurityType::TLS:
				UseTLS_ = true;
				break;
			case SecurityType::No:
				break;
			}

			InSecurityRequired_ = dia->GetInSecurityRequired ();
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
		}

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

	QMutex* Account::GetMutex () const
	{
		return AccMutex_;
	}

	QString Account::BuildInURL ()
	{
		QMutexLocker l (GetMutex ());

		QString result;

		switch (InType_)
		{
		case InType::IMAP:
			result = UseSSL_ ? "imaps://" : "imap://";
			break;
		case InType::POP3:
			result = "pop3://";
			break;
		case InType::Maildir:
			result = "maildir://localhost";
			break;
		}

		if (InType_ != InType::Maildir)
		{
			result += Login_;
			result += ":";
			result.replace ('@', "%40");

			QString pass;
			getPassword (&pass);

			result += pass + '@';
		}

		result += InHost_;

		qDebug () << Q_FUNC_INFO << result;

		return result;
	}

	QString Account::BuildOutURL ()
	{
		QMutexLocker l (GetMutex ());

		if (OutType_ == OutType::Sendmail)
			return "sendmail://localhost";

		QString result = OutSecurity_ == SecurityType::SSL ? "smtps://" : "smtp://";

		if (SMTPNeedsAuth_)
		{
			QString pass;
			if (OutLogin_.isEmpty ())
			{
				result += Login_;
				getPassword (&pass);
			}
			else
			{
				result += OutLogin_;
				getPassword (&pass, Direction::Out);
			}
			result += ":" + pass;

			result.replace ('@', "%40");
			result += '@';
		}

		result += OutHost_;

		qDebug () << Q_FUNC_INFO << result;

		return result;
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

	void Account::buildInURL (QString *res)
	{
		*res = BuildInURL ();
	}

	void Account::buildOutURL (QString *res)
	{
		*res = BuildOutURL ();
	}

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

		MailModelMgr_->appendMessages (messages);
	}

	void Account::handleGotUpdatedMessages (QList<Message_ptr> messages)
	{
		Core::Instance ().GetStorage ()->SaveMessages (this, messages);
		emit mailChanged ();

		MailModelMgr_->appendMessages (messages);
	}

	void Account::handleGotOtherMessages (QList<QByteArray> ids, QStringList folder)
	{
		qDebug () << Q_FUNC_INFO << ids.size ();
		QList<Message_ptr> msgs;
		Q_FOREACH (auto id, ids)
			msgs << Core::Instance ().GetStorage ()->LoadMessage (this, id);

		MailModelMgr_->appendMessages (msgs);
	}

	namespace
	{
		QStandardItem* BuildFolderItem (QStringList folder, QStandardItem *root)
		{
			if (folder.isEmpty ())
				return root;

			const QString name = folder.takeFirst ();
			for (int i = 0; i < root->rowCount (); ++i)
				if (root->child (i)->text () == name)
					return BuildFolderItem (folder, root->child (i));

			QStandardItem *item = new QStandardItem (name);
			root->appendRow (item);
			return BuildFolderItem (folder, item);
		}
	}

	void Account::handleGotFolders (QList<QStringList> folders)
	{
		FolderManager_->SetFolders (folders);

		FoldersModel_->clear ();
		Q_FOREACH (const QStringList& folder, folders)
		{
			auto item = BuildFolderItem (folder, FoldersModel_->invisibleRootItem ());
			item->setData (folder, FoldersRole::Path);
		}
	}

	void Account::handleMessageBodyFetched (Message_ptr msg)
	{
		Core::Instance ().GetStorage ()->SaveMessages (this, { msg });
		emit messageBodyFetched (msg);
	}
}
}
