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
#include "ljaccountconfigurationwidget.h"
#include "ljaccountconfigurationdialog.h"
#include "ljbloggingplatform.h"
#include "ljxmlrpc.h"
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LJAccount::LJAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (qobject_cast<LJBloggingPlatform*> (parent))
	, LJXmlRpc_ (new LJXmlRPC (this))
	, Name_ (name)
	{
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

	void LJAccount::FillSettings (LJAccountConfigurationWidget *widget)
	{
		Login_ = widget->GetLogin ();
		const QString& pass = widget->GetPassword ();
		if (!pass.isNull ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + GetAccountID (),
					&Core::Instance ());

		emit accountSettingsChanged ();
	}

	QByteArray LJAccount::Serialize () const
	{
		quint16 ver = 1;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< Name_
					<< Login_;
		}

		return result;
	}

	LJAccount* LJAccount::Deserialize (const QByteArray& data, QObject *parent)
	{
		quint16 ver = 0;
		QDataStream in (data);
		in >> ver;

		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		QString name;
		in >> name;
		LJAccount *result = new LJAccount (name, parent);
		in >> result->Login_;

		return result;
	}

	void LJAccount::Validate ()
	{
		QString key ("org.LeechCraft.Blogique.PassForAccount/" + GetAccountID ());
		QString pass = Util::GetPassword (key,
				QString (),
				&Core::Instance ());

		LJXmlRpc_->Validate (Login_, pass);
	}

}
}
}
