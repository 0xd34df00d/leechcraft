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
#include <vmime/security/defaultAuthenticator.hpp>
#include <vmime/net/transport.hpp>
#include <vmime/net/store.hpp>
#include <util/util.h>
#include "core.h"
#include "accountconfigdialog.h"

namespace LeechCraft
{
namespace Snails
{
	namespace
	{
		class VMimeAuth : public vmime::security::defaultAuthenticator
		{
			Account::Direction Dir_;
			Account *Acc_;
		public:
			VMimeAuth (Account::Direction, Account*);

			const vmime::string getUsername () const;
			const vmime::string getPassword () const;
		private:
			QByteArray GetID () const
			{
				QByteArray id = "org.LeechCraft.Snails.PassForAccount/" + Acc_->GetID ();
				id += Dir_ == Account::DOut ? "/Out" : "/In";
				return id;
			}

			QString GetPassImpl () const;
		};

		VMimeAuth::VMimeAuth (Account::Direction dir, Account *acc)
		: Dir_ (dir)
		, Acc_ (acc)
		{
		}

		const vmime::string VMimeAuth::getUsername () const
		{
			switch (Dir_)
			{
			case Account::DOut:
				return Acc_->GetOutUsername ().toUtf8 ().constData ();
			default:
				return Acc_->GetInUsername ().toUtf8 ().constData ();
			}
		}

		const vmime::string VMimeAuth::getPassword () const
		{
			QString pass = GetPassImpl ();
			if (!pass.isEmpty ())
				return pass.toUtf8 ().constData ();

			pass = QInputDialog::getText (0,
					"LeechCraft",
					Account::tr ("Enter password for account %1:")
							.arg (Acc_->GetName ()),
					QLineEdit::Password);
			if (pass.isEmpty ())
				return vmime::string ();

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

			return pass.toUtf8 ().constData ();
		}

		QString VMimeAuth::GetPassImpl () const
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
	}

	Account::Account (QObject *parent)
	: QObject (parent)
	, Session_ (vmime::create<vmime::net::session> ())
	, ID_ (QUuid::createUuid ().toByteArray ())
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
		return InType_ == ITMaildir ?
			QString () :
			InHost_ + ':' + QString::number (InPort_);
	}

	QString Account::GetType () const
	{
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

	QByteArray Account::Serialize () const
	{
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

		RebuildSessConfig ();
	}

	void Account::OpenConfigDialog ()
	{
		std::unique_ptr<AccountConfigDialog> dia (new AccountConfigDialog);

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

		if (dia->exec () != QDialog::Accepted)
			return;

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

		RebuildSessConfig ();
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

	void Account::RebuildSessConfig ()
	{
		Session_->getProperties ().removeAllProperties ();

		vmime::string prefix;
		switch (InType_)
		{
		case ITIMAP:
			prefix = "store.imap.";
			break;
		case ITPOP3:
			prefix = "store.pop3.";
			Session_->getProperties () [prefix + "options.apop"] = APOP_;
			Session_->getProperties () [prefix + "options.apop.fallback"] = APOPFail_;
			break;
		case ITMaildir:
			prefix = "store.maildir.";
			break;
		}

		Session_->getProperties () [prefix + "options.sasl"] = UseSASL_;
		Session_->getProperties () [prefix + "options.sasl.fallback"] = SASLRequired_;
		Session_->getProperties () [prefix + "connection.tls"] = UseTLS_;
		Session_->getProperties () [prefix + "connection.tls.required"] = TLSRequired_;
		Session_->getProperties () [prefix + "server.address"] = InHost_.toUtf8 ().constData ();
		Session_->getProperties () [prefix + "server.port"] = InPort_;
		Session_->getProperties () [prefix + "server.rootpath"] = InHost_.toUtf8 ().constData ();

		vmime::string opref;
		switch (OutType_)
		{
		case OTSMTP:
			opref = "transport.smtp.";
			Session_->getProperties () [opref + "options.need-authentication"] = SMTPNeedsAuth_;
			break;
		case OTSendmail:
			opref = "transport.sendmail.";
			break;
		}
		Session_->getProperties () [opref + "server.address"] = OutHost_.toUtf8 ().constData ();
		Session_->getProperties () [opref + "server.port"] = OutPort_;
	}

	vmime::utility::ref<vmime::net::store> Account::MakeStore ()
	{
		return Session_->getStore (vmime::utility::url (BuildInURL ().toUtf8 ().constData ()),
				vmime::create<VMimeAuth> (DIn, this));
	}

	vmime::utility::ref<vmime::net::transport> Account::MakeTransport ()
	{
		return Session_->getTransport (vmime::utility::url (BuildOutURL ().toUtf8 ().constData ()),
				vmime::create<VMimeAuth> (DOut, this));
	}

	QString Account::BuildInURL () const
	{
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

		result += InHost_;

		if (InType_ != ITMaildir)
			result += ":" + QString::number (InPort_);

		return result;
	}

	QString Account::BuildOutURL () const
	{
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
}
}
