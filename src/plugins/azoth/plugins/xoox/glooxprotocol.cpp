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

#include "glooxprotocol.h"
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/iprotocolplugin.h>
#include "glooxaccount.h"
#include "core.h"
#include "joingroupchatwidget.h"
#include "glooxaccountconfigurationwidget.h"
#include "bookmarkeditwidget.h"
#include "inbandaccountregfirstpage.h"

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

	QByteArray GlooxProtocol::GetProtocolID () const
	{
		return "Xoox.Gloox.XMPP";
	}
	
	QList<QWidget*> GlooxProtocol::GetAccountRegistrationWidgets (AccountAddOptions options)
	{
		QList<QWidget*> result;
		if (options & AAORegisterNewAccount)
		{
			result << new InBandAccountRegFirstPage ();
		}
		else
			result << new GlooxAccountConfigurationWidget ();
		return result;
	}
	
	void GlooxProtocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		GlooxAccountConfigurationWidget *w =
				qobject_cast<GlooxAccountConfigurationWidget*> (widgets.value (0));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}
		
		GlooxAccount *account = new GlooxAccount (name, this);
		account->FillSettings (w);
		Accounts_ << account;
		saveAccounts ();
		emit accountAdded (account);
	}

	QWidget* GlooxProtocol::GetMUCJoinWidget ()
	{
		return new JoinGroupchatWidget ();
	}
	
	QWidget* GlooxProtocol::GetMUCBookmarkEditorWidget ()
	{
		return new BookmarkEditWidget ();
	}

	void GlooxProtocol::RemoveAccount (QObject *acc)
	{
		GlooxAccount *accObj = qobject_cast<GlooxAccount*> (acc);
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
			QString name = queryItems ["name"];
			QStringList groups (queryItems ["group"]);
			acc->AddEntry (path, name, groups);
			if (queryItems.contains ("subscribe"))
				acc->RequestAuth (path, QString (), name, groups);
		}
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
}
}
}
