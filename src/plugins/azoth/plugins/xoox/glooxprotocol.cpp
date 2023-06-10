/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glooxprotocol.h"
#include <QInputDialog>
#include <QMainWindow>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QtDebug>
#include <QXmppLogger.h>
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include <interfaces/azoth/iprotocolplugin.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "glooxaccount.h"
#include "core.h"
#include "joingroupchatwidget.h"
#include "glooxaccountconfigurationwidget.h"
#include "inbandaccountregfirstpage.h"
#include "inbandaccountregsecondpage.h"
#include "inbandaccountregthirdpage.h"
#include "clientconnection.h"
#include "accountsettingsholder.h"
#include "roomclentry.h"
#include "roomhandler.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	GlooxProtocol::GlooxProtocol (CapsDatabase *capsDB, VCardStorage *storage, QObject *parent)
	: QObject { parent }
	, ParentProtocolPlugin_ { parent }
	, CapsDB_ { capsDB }
	, VCardStorage_ { storage }
	{
		const auto logger = QXmppLogger::getLogger ();
		logger->setLoggingType (QXmppLogger::FileLogging);
		logger->setLogFilePath (Util::CreateIfNotExists ("azoth").filePath ("qxmpp.log"));
		logger->setMessageTypes (QXmppLogger::AnyMessage);
	}

	GlooxProtocol::~GlooxProtocol ()
	{
		for (auto acc : Accounts_)
		{
			acc->Release ();
			emit accountRemoved (acc);
		}
	}

	void GlooxProtocol::Prepare ()
	{
		RestoreAccounts ();
	}

	IProxyObject* GlooxProtocol::GetProxyObject () const
	{
		return ProxyObject_;
	}

	void GlooxProtocol::SetProxyObject (IProxyObject *po)
	{
		ProxyObject_ = po;
	}

	CapsDatabase* GlooxProtocol::GetCapsDatabase () const
	{
		return CapsDB_;
	}

	VCardStorage* GlooxProtocol::GetVCardStorage () const
	{
		return VCardStorage_;
	}

	QObject* GlooxProtocol::GetQObject ()
	{
		return this;
	}

	IProtocol::ProtocolFeatures GlooxProtocol::GetFeatures() const
	{
		return PFSupportsMUCs | PFMUCsJoinable | PFSupportsInBandRegistration;
	}

	QList<QObject*> GlooxProtocol::GetRegisteredAccounts ()
	{
		return Util::Map (Accounts_, Util::Upcast<QObject*>);
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
		static QIcon icon ("lcicons:/plugins/azoth/plugins/xoox/resources/images/jabbericon.svg");
		return icon;
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
		auto w = qobject_cast<GlooxAccountConfigurationWidget*> (widgets.value (pos));
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "got invalid widgets"
					<< widgets;
			return;
		}

		GlooxAccount *account = new GlooxAccount (name, this, this);
		account->GetSettings ()->FillSettings (w);

		if (isNewAcc)
		{
			if (const auto second = qobject_cast<InBandAccountRegSecondPage*> (widgets.value (1)))
				ProxyObject_->SetPassword (second->GetPassword (), account);
		}
		else
		{
			const auto& pass = w->GetPassword ();
			if (!pass.isNull ())
				ProxyObject_->SetPassword (pass, account);
		}

		Accounts_ << account;

		account->Init ();

		saveAccounts ();
		emit accountAdded (account);
	}

	void GlooxProtocol::RemoveAccount (QObject *acc)
	{
		const auto accObj = qobject_cast<GlooxAccount*> (acc);

		accObj->Release ();
		Accounts_.removeAll (accObj);
		emit accountRemoved (accObj);

		accObj->deleteLater ();
		saveAccounts ();
	}

	QWidget* GlooxProtocol::GetMUCJoinWidget ()
	{
		return new JoinGroupchatWidget ();
	}

	QVariantMap GlooxProtocol::TryGuessMUCIdentifyingData (const QString& text, QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto acc = qobject_cast<GlooxAccount*> (entry->GetParentAccount ()->GetQObject ());

		QVariantMap result;
		result ["AccountID"] = acc->GetAccountID ();
		result ["Nick"] = acc->GetNick ();
		result ["Password"] = text.section (' ', 1);

		const auto& address = text.section (' ', 0, 0);
		if (address.contains ('@'))
		{
			result ["Room"] = address.section ('@', 0, 0);
			result ["Server"] = address.section ('@', 1);
		}
		else
		{
			result ["Room"] = address;

			auto& server = result ["Server"];
			switch (entry->GetEntryType ())
			{
			case ICLEntry::EntryType::MUC:
			{
				const auto rh = qobject_cast<RoomCLEntry*> (entryObj)->GetRoomHandler ();
				server = rh->GetRoomJID ().section ('@', 1);
				break;
			}
			case ICLEntry::EntryType::PrivateChat:
			{
				const auto roomEntry = entry->GetParentCLEntryObject ();
				const auto rh = qobject_cast<RoomCLEntry*> (roomEntry)->GetRoomHandler ();
				server = rh->GetRoomJID ().section ('@', 1);
				break;
			}
			default:
			{
				const auto& serverStr = acc->GetSettings ()->GetJID ().section ('@', 1);
				server = "conference." + serverStr;
			}
			}
		}

		return result;
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
		const auto& queryParts = url.query (QUrl::FullyEncoded).split (';');
		for (const auto& part : queryParts)
		{
			const auto& split = part.split ('=');
			if (split.size () > 2)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrect query part"
						<< part
						<< split;
				continue;
			}
			queryItems [split.at (0)] = QUrl::fromPercentEncoding (split.value (1).toLatin1 ());
		}

		const QString& path = url.path ();
		if (queryItems.contains ("join"))
		{
			const QStringList& split = path.split ('@', Qt::SkipEmptyParts);
			if (split.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
						<< "incorrect room format"
						<< path
						<< split;
				return;
			}
			acc->JoinRoom (split.at (1), split.at (0), acc->GetNick (), {});
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

			auto [jid, variant] = ClientConnection::Split (path);
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

			ProxyObject_->OpenChat (entry->GetEntryID (), acc->GetAccountID (), body, variant);
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
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Azoth",
						tr ("Unable to import account: malformed import data."),
						Priority::Critical));
			return false;
		}

		for (auto acc : Accounts_)
			if (acc->GetAccountName () == name)
			{
				GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Azoth",
							tr ("Account %1 already exists, cannot import another one."),
							Priority::Critical));
				return false;
			}

		// Maybe a kludge, dunno. Don't beat me hard :(
		GlooxAccountConfigurationWidget w;
		w.SetJID (info ["Jid"].toString ());
		w.SetHost (info ["Host"].toString ());
		w.SetPort (info ["Port"].toInt ());
		w.SetNick (info ["Nick"].toString ());

		GlooxAccount *account = new GlooxAccount (name, this, this);
		account->GetSettings ()->FillSettings (&w);

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
