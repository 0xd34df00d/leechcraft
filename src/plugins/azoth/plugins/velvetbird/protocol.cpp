/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "protocol.h"
#include <QIcon>
#include <QFormLayout>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>
#include "account.h"
#include "accregfirstpage.h"

namespace LC
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

	QObject* Protocol::GetQObject()
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
			result = QIcon ("lcicons:/azoth/velvetbird/resources/images/velvetbird.svg");
		return result;
	}

	QByteArray Protocol::GetProtocolID () const
	{
		return "VelvetBird." + GetPurpleID ();
	}

	QList<QWidget*> Protocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions)
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
		purple_account_set_alias (pacc, nameWidget->GetNick ().toUtf8 ().constData ());
		purple_account_set_string (pacc, "AccountName", name.toUtf8 ().constData ());
		purple_accounts_add (pacc);

		PushAccount (pacc);
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
		auto account = new Account (pacc, this);
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
