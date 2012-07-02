/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <memory>
#include "ljaccount.h"
#include <QtDebug>
#include <util/passutils.h>
#include <util/util.h>
#include "core.h"
#include "ljaccountconfigurationwidget.h"
#include "ljaccountconfigurationdialog.h"
#include "ljbloggingplatform.h"
#include "ljprofile.h"
#include "ljxmlrpc.h"
#include "profilewidget.h"
#include "utils.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJAccount::LJAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (qobject_cast<LJBloggingPlatform*> (parent))
	, LJXmlRpc_ (new LJXmlRPC (this, this))
	, Name_ (name)
	, IsValidated_ (false)
	, LJProfile_ (std::make_shared<LJProfile> (this))
	{
		qRegisterMetaType<LJProfileData> ("LJProfileData");
		qRegisterMetaTypeStreamOperators<QList<LJFriendGroup>> ("QList<LJFriendGroup>");
		qRegisterMetaTypeStreamOperators<QList<LJMood>> ("QList<LJMood>");

		connect (LJXmlRpc_,
				SIGNAL (validatingFinished (bool)),
				this,
				SLOT (handleValidatingFinished (bool)));
		connect (LJXmlRpc_,
				SIGNAL (error (int, const QString&)),
				this,
				SLOT (handleXmlRpcError (int, const QString&)));
		connect (LJXmlRpc_,
				SIGNAL (profileUpdated (const LJProfileData&)),
				LJProfile_.get (),
				SLOT (handleProfileUpdate (const LJProfileData&)));
	}

	QObject* LJAccount::GetObject ()
	{
		return this;
	}

	QObject* LJAccount::GetParentBloggingPlatform () const
	{
		return ParentBloggingPlatform_;
	}

	QString LJAccount::GetAccountName () const
	{
		return Name_;
	}

	QString LJAccount::GetOurLogin () const
	{
		//TODO
		return QString ();
	}

	void LJAccount::RenameAccount (const QString& name)
	{

	}

	QByteArray LJAccount::GetAccountID () const
	{
		return ParentBloggingPlatform_->GetBloggingPlatformID () + "_" +
				Login_.toUtf8();
	}

	void LJAccount::OpenConfigurationDialog ()
	{
		std::unique_ptr<LJAccountConfigurationDialog> dia (new LJAccountConfigurationDialog (0));

		if (!Login_.isEmpty ())
			dia->ConfWidget ()->SetLogin (Login_);

		QString key ("org.LeechCraft.Blogique.PassForAccount/" + GetAccountID ());
		dia->ConfWidget ()->SetPassword (Util::GetPassword (key,
				QString (),
				&Core::Instance ()));

		if (dia->exec () == QDialog::Rejected)
			return;

		FillSettings (dia->ConfWidget ());
	}

	bool LJAccount::IsValidated () const
	{
		return IsValidated_;
	}

	QObject* LJAccount::GetProfile ()
	{
		return LJProfile_.get ();
	}

	void LJAccount::FillSettings (LJAccountConfigurationWidget *widget)
	{
		Login_ = widget->GetLogin ();
		const QString& pass = widget->GetPassword ();
		if (!pass.isNull ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + GetAccountID (),
					&Core::Instance ());

		emit accountSettingsChanged ();
		Validate ();
	}

	QByteArray LJAccount::Serialize () const
	{
		quint16 ver = 2;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< Name_
					<< Login_
					<< IsValidated_
					<< LJProfile_->GetProfileData ();
		}

		return result;
	}

	LJAccount* LJAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 ver = 0;
		QDataStream in (data);
		in >> ver;

		if (ver > 2 ||
				ver < 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		QString name;
		in >> name;
		LJAccount *result = new LJAccount (name, parent);
		in >> result->Login_
				>> result->IsValidated_;

		if (ver == 2)
		{
			LJProfileData profile;
			in >> profile;
			result->LJProfile_->handleProfileUpdate (profile);
		}

		return result;
	}

	void LJAccount::Validate ()
	{
		QString key ("org.LeechCraft.Blogique.PassForAccount/" + GetAccountID ());
		const QString& pass = Util::GetPassword (key,
				QString (),
				&Core::Instance ());

		LJXmlRpc_->Validate (Login_, pass);
	}

	void LJAccount::Init ()
	{
		connect (this,
				SIGNAL (accountValidated (bool)),
				ParentBloggingPlatform_,
				SLOT (handleAccountValidated (bool)));

		connect (this,
				SIGNAL (accountSettingsChanged ()),
				ParentBloggingPlatform_,
				SLOT (saveAccounts ()));
	}

	void LJAccount::AddFriends (const QSet<LJFriendEntry_ptr>& friends)
	{
		LJProfile_->AddFriends (friends);
	}

	void LJAccount::handleValidatingFinished (bool success)
	{
		IsValidated_ = success;
		qDebug () << Q_FUNC_INFO
				<< "account"
				<< GetAccountID ()
				<< "validating result is"
				<< IsValidated_;

		emit accountValidated (IsValidated_);
		emit accountSettingsChanged ();
	}

	void LJAccount::handleXmlRpcError (int errorCode, const QString& msgInEng)
	{
		Entity e = Util::MakeNotification ("Blogique",
				tr ("%1 (original message: %2)")
						.arg (MetidaUtils::GetLocalizedErrorMessage (errorCode), msgInEng),
				PWarning_);

		qWarning () << Q_FUNC_INFO
				<< "error code:"
				<< errorCode
				<< "error text:"
				<< msgInEng;

		Core::Instance ().SendEntity (e);
	}

}
}
}
