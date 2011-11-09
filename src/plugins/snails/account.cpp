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

#include "account.h"
#include <stdexcept>
#include <QUuid>
#include <QDataStream>
#include <QInputDialog>
#include <QMutex>
#include <util/util.h>
#include "core.h"
#include "accountconfigdialog.h"
#include "accountthread.h"
#include "accountthreadworker.h"
#include "storage.h"

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
	, TLSRequired_ (false)
	, SMTPNeedsAuth_ (true)
	, APOP_ (false)
	, APOPFail_ (false)
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
		return InType_ == ITMaildir ?
			QString () :
			InHost_ + ':' + QString::number (InPort_);
	}

	QString Account::GetType () const
	{
		QMutexLocker l (GetMutex ());
		switch (InType_)
		{
		case ITIMAP:
			return "IMAP";
		case ITPOP3:
			return "POP3";
		case ITMaildir:
			return "Maildir";
		default:
			return "<unknown>";
		}
	}

	void Account::FetchNewHeaders (int from)
	{
		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"fetchNewHeaders",
				Qt::QueuedConnection,
				Q_ARG (int, from));
	}

	void Account::FetchWholeMessage (const QByteArray& id)
	{
		QMetaObject::invokeMethod (Thread_->GetWorker (),
				"fetchWholeMessage",
				Qt::QueuedConnection,
				Q_ARG (QByteArray, id));
	}

	QByteArray Account::Serialize () const
	{
		QMutexLocker l (GetMutex ());

		QByteArray result;

		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (1);
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
			<< static_cast<quint8> (OutType_);

		return result;
	}

	void Account::Deserialize (const QByteArray& arr)
	{
		QDataStream in (arr);
		quint8 version = 0;
		in >> version;

		if (version != 1)
			throw std::runtime_error (qPrintable ("Unknown version " + QString::number (version)));

		quint8 inType = 0, outType = 0;

		{
			QMutexLocker l (GetMutex ());
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
		}
	}

	void Account::OpenConfigDialog ()
	{
		std::unique_ptr<AccountConfigDialog> dia (new AccountConfigDialog);

		{
			QMutexLocker l (GetMutex ());
			dia->SetName (AccName_);
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
		}

		if (dia->exec () != QDialog::Accepted)
			return;

		{
			QMutexLocker l (GetMutex ());
			AccName_ = dia->GetName ();
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
		case ITIMAP:
			result = "imap://";
			break;
		case ITPOP3:
			result = "pop3://";
			break;
		case ITMaildir:
			result = "maildir://localhost";
			break;
		}

		if (InType_ != ITMaildir)
		{
			result += Login_;
			result += ":";

			QString pass;
			getPassword (&pass);

			result += pass + '@';
		}

		result += InHost_;

		// TODO
		//if (InType_ != ITMaildir)
			//result += ":" + QString::number (InPort_);

		qDebug () << Q_FUNC_INFO << result;

		return result;
	}

	QString Account::BuildOutURL ()
	{
		QMutexLocker l (GetMutex ());

		QString result;

		switch (OutType_)
		{
		case OTSMTP:
			result = "smtp://";
			result += OutHost_ + ":" + QString::number (OutPort_);
			break;
		case OTSendmail:
			result = "sendmail://localhost";
			break;
		}

		return result;
	}

	QString Account::GetPassImpl ()
	{
		QList<QVariant> keys;
		keys << GetID ();
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

	void Account::buildInURL (QString *res)
	{
		*res = BuildInURL ();
	}

	void Account::buildOutURL (QString *res)
	{
		*res = BuildOutURL ();
	}

	void Account::getPassword (QString *outPass)
	{
		QString pass = GetPassImpl ();
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
		keys << GetID ();

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
}
}
