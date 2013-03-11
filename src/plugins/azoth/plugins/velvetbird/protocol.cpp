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

#include "protocol.h"
#include <QIcon>
#include <QFormLayout>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>
#include "account.h"
#include "accregfirstpage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	Protocol::Protocol (PurplePlugin *plug, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, PPlug_ (plug)
	{
	}

	void Protocol::Release ()
	{
		for (auto acc : Accounts_)
		{
			acc->Release ();
			emit accountRemoved (acc);
		}
	}

	QObject* Protocol::GetObject()
	{
		return this;
	}

	IProtocol::ProtocolFeatures Protocol::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> Protocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QObject* Protocol::GetParentProtocolPlugin () const
	{
		return parent ();
	}

	QString Protocol::GetProtocolName () const
	{
		return QString::fromUtf8 (purple_plugin_get_name (PPlug_)) + " (by libpurple)";
	}

	QIcon Protocol::GetProtocolIcon () const
	{
		auto id = GetPurpleID ();
		if (id.startsWith ("prpl-"))
			id.remove (0, 5);

		QIcon result = QIcon::fromTheme (QString::fromUtf8 ("im-" + id));
		if (result.isNull ())
			result = QIcon (":/azoth/velvetbird/resources/images/velvetbird.svg");
		return result;
	}

	QByteArray Protocol::GetProtocolID () const
	{
		return "VelvetBird." + GetPurpleID ();
	}

	QList<QWidget*> Protocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions opts)
	{
		auto nameWidget = new AccRegFirstPage ();
		return { nameWidget };
	}

	void Protocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		auto nameWidget = dynamic_cast<AccRegFirstPage*> (widgets.value (0));
		if (!nameWidget)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect widgets"
					<< widgets;
			return;
		}

		auto pacc = purple_account_new (nameWidget->GetName ().toUtf8 ().constData (),
				GetPurpleID ().constData ());
		purple_account_set_alias (pacc, name.toUtf8 ().constData ());
		purple_accounts_add (pacc);

		PushAccount (pacc);
	}

	QWidget* Protocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void Protocol::RemoveAccount (QObject *accObj)
	{
		auto acc = qobject_cast<Account*> (accObj);
		emit accountRemoved (accObj);

		purple_accounts_delete (acc->GetPurpleAcc ());
		delete acc;
	}

	QByteArray Protocol::GetPurpleID () const
	{
		return purple_plugin_get_id (PPlug_);
	}

	void Protocol::PushAccount (PurpleAccount *pacc)
	{
		const auto& name = QString::fromUtf8 (purple_account_get_alias (pacc));
		auto account = new Account (name, pacc, this);
		Accounts_ << account;
		emit accountAdded (account);

		pacc->ui_data = account;
	}

	ICoreProxy_ptr Protocol::GetCoreProxy () const
	{
		return Proxy_;
	}
}
}
}
