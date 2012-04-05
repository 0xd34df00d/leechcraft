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

#include "glooxprotocol.h"
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/azoth/iprotocolplugin.h>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccount.h"
#include "core.h"
#include "joingroupchatwidget.h"
#include "glooxaccountconfigurationwidget.h"
#include "inbandaccountregfirstpage.h"
#include "inbandaccountregsecondpage.h"
#include "inbandaccountregthirdpage.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	GlooxProtocol::GlooxProtocol (QObject *parent)
	: QObject (parent)
	, ParentProtocolPlugin_ (parent)
	, ProxyObject_ (0)
	{
	}

	GlooxProtocol::~GlooxProtocol ()
	{
		Q_FOREACH (QObject *acc, GetRegisteredAccounts ())
			emit accountRemoved (acc);
	}

	void GlooxProtocol::Prepare ()
	{
		RestoreAccounts ();
	}

	QObject* GlooxProtocol::GetProxyObject () const
	{
		return ProxyObject_;
	}

	void GlooxProtocol::SetProxyObject (QObject *po)
	{
		ProxyObject_ = po;
	}

	QObject* GlooxProtocol::GetObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures GlooxProtocol::GetFeatures() const
	{
		return PFSupportsMUCs | PFMUCsJoinable | PFSupportsInBandRegistration;
	}

	QList<QObject*> GlooxProtocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		Q_FOREACH (GlooxAccount *acc, Accounts_)
			result << acc;
		return result;
	}

	QObject* GlooxProtocol::GetParentProtocolPlugin () const
	{
		return ParentProtocolPlugin_;
	}

	QString GlooxProtocol::GetProtocolName () const
	{
		return "XMPP";
	}

	QIcon GlooxProtocol::GetProtocolIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/xoox/resources/images/jabbericon.svg");
	}

	QByteArray GlooxProtocol::GetProtocolID () const
	{
		return "Xoox.Gloox.XMPP";
	}

	QList<QWidget*> GlooxProtocol::GetAccountRegistrationWidgets (AccountAddOptions options)
	{
		QList<QWidget*> result;
		if (options & AAORegisterNewAccount)
		{
			InBandAccountRegFirstPage *first = new InBandAccountRegFirstPage ();
			InBandAccountRegSecondPage *second = new InBandAccountRegSecondPage (first);
			InBandAccountRegThirdPage *third = new InBandAccountRegThirdPage (second);
			GlooxAccountConfigurationWidget *fourth = new GlooxAccountConfigurationWidget ();
			third->SetConfWidget (fourth);
			result << first;
			result << second;
			result << third;
			result << fourth;
		}
		else
			result << new GlooxAccountConfigurationWidget ();

		result.at (0)->setProperty ("IsNewAccount",
				static_cast<bool> (options & AAORegisterNewAccount));

		return result;
	}

	void GlooxProtocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		if (!widgets.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty widgets set";
			return;
		}

		const bool isNewAcc = widgets.at (0)->property ("IsNewAccount").toBool ();
		const int pos = isNewAcc ? 3 : 0;
		GlooxAccountConfigurationWidget *w =
				qobject_cast<GlooxAccountConfigurationWidget*> (widgets.value (pos));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		GlooxAccount *account = new GlooxAccount (name, this);
		account->FillSettings (w);

		if (isNewAcc)
		{
			InBandAccountRegSecondPage *second =
				qobject_cast<InBandAccountRegSecondPage*> (widgets.value (1));
			if (second)
				qobject_cast<IProxyObject*> (ProxyObject_)->
						SetPassword (second->GetPassword (), account);
		}
		else
		{
			const QString& pass = w->GetPassword ();
			if (!pass.isNull ())
				qobject_cast<IProxyObject*> (ProxyObject_)->
						SetPassword (pass, account);
		}

		Accounts_ << account;

		account->Init ();

		saveAccounts ();
		emit accountAdded (account);
	}

	QWidget* GlooxProtocol::GetMUCJoinWidget ()
	{
		return new JoinGroupchatWidget ();
	}

	void GlooxProtocol::RemoveAccount (QObject *acc)
	{
		GlooxAccount *accObj = qobject_cast<GlooxAccount*> (acc);

		QMetaObject::invokeMethod (accObj,
				"removedCLItems",
				Q_ARG (QList<QObject*>, accObj->GetCLEntries ()));

		Accounts_.removeAll (accObj);
		emit accountRemoved (accObj);

		accObj->deleteLater ();
		saveAccounts ();
	}

	bool GlooxProtocol::SupportsURI (const QUrl& url) const
	{
		return url.scheme () == "xmpp";
	}

	void GlooxProtocol::HandleURI (const QUrl& url, QObject *accountObj)
	{
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (accountObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accountObj
					<< "isn't GlooxAccount";
			return;
		}

		QMap<QString, QString> queryItems;
		QList<QByteArray> queryParts = url.encodedQuery ().split (';');
		Q_FOREACH (const QByteArray& part, queryParts)
		{
			const QList<QByteArray>& splitted = part.split ('=');
			if (splitted.size () > 2)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrect query part"
						<< part
						<< splitted;
				continue;
			}
			queryItems [splitted.at (0)] = QUrl::fromPercentEncoding (splitted.value (1));
		}

		qDebug () << "HANDLE" << queryItems;

		const QString& path = url.path ();
		if (queryItems.contains ("join"))
		{
			const QStringList& split = path.split ('@', QString::SkipEmptyParts);
			if (split.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrect room format"
						<< path
						<< split;
				return;
			}
			acc->JoinRoom (split.at (1), split.at (0), acc->GetNick ());
		}
		else if (queryItems.contains ("roster") ||
				queryItems.contains ("subscribe"))
		{
			const QString& name = queryItems ["name"];
			const QStringList groups (queryItems ["group"]);
			acc->AddEntry (path, name, groups);
			if (queryItems.contains ("subscribe"))
				acc->RequestAuth (path, QString (), name, groups);
		}
		else if (queryItems.contains ("message"))
		{
			const QString& body = queryItems ["body"];

			QString jid;
			QString variant;
			ClientConnection::Split (path, &jid, &variant);
			if (jid.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty jid for path"
						<< path
						<< url.toString ();
				return;
			}

			QObject *entryObj = acc->GetClientConnection ()->
					GetCLEntry (jid, variant);
			if (!entryObj)
			{
				qWarning () << Q_FUNC_INFO
						<< "entry for the given jid not found"
						<< jid
						<< variant;
				return;
			}
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

			Core::Instance ().GetPluginProxy ()->OpenChat (entry->GetEntryID (),
					acc->GetAccountID (), body, variant);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unhandled query items"
					<< queryItems;
	}

	QString GlooxProtocol::GetImportProtocolID () const
	{
		return "xmpp";
	}

	bool GlooxProtocol::ImportAccount (const QVariantMap& info)
	{
		const QString& name = info ["Name"].toString ();

		if (name.isEmpty () ||
				info ["Jid"].toString ().isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "malformed import info"
					<< info;
			Core::Instance ().SendEntity (Util::MakeNotification ("Azoth",
						tr ("Unable to import account: malformed import data."),
						PCritical_));
			return false;
		}

		Q_FOREACH (GlooxAccount *acc, Accounts_)
			if (acc->GetAccountName () == name)
			{
				Core::Instance ().SendEntity (Util::MakeNotification ("Azoth",
							tr ("Account %1 already exists, cannot import another one."),
							PCritical_));
				return false;
			}

		// Maybe a kludge, dunno. Don't beat me hard :(
		GlooxAccountConfigurationWidget w;
		w.SetJID (info ["Jid"].toString ());
		w.SetHost (info ["Host"].toString ());
		w.SetPort (info ["Port"].toInt ());
		w.SetNick (info ["Nick"].toString ());

		GlooxAccount *account = new GlooxAccount (name, this);
		account->FillSettings (&w);

		Accounts_ << account;
		account->Init ();
		saveAccounts ();
		emit accountAdded (account);

		return true;
	}

	QString GlooxProtocol::GetEntryID (const QString& hrID, QObject *accObj)
	{
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "passed object is not a GlooxAccount"
					<< accObj;
			return QString ();
		}

		return acc->GetAccountID () + '_' + hrID;
	}

	void GlooxProtocol::RestoreAccounts ()
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Xoox_Accounts");
		int size = settings.beginReadArray ("Accounts");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QByteArray data = settings.value ("SerializedData").toByteArray ();
			GlooxAccount *acc = GlooxAccount::Deserialize (data, this);
			if (!acc)
			{
				qWarning () << Q_FUNC_INFO
						<< "unserializable acount"
						<< i;
				continue;
			}

			connect (acc,
					SIGNAL (accountSettingsChanged ()),
					this,
					SLOT (saveAccounts ()));

			Accounts_ << acc;

			emit accountAdded (acc);
		}
		settings.endArray ();
	}

	void GlooxProtocol::saveAccounts () const
	{
		QSettings settings (QSettings::IniFormat, QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Xoox_Accounts");
		settings.beginWriteArray ("Accounts");
		for (int i = 0, size = Accounts_.size ();
				i < size; ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("SerializedData", Accounts_.at (i)->Serialize ());
		}
		settings.endArray ();
		settings.sync ();
	}
}
}
}
