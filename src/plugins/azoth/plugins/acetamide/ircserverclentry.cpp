/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircserverclentry.h"
#include <QAction>
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircserverhandler.h"
#include "clientconnection.h"
#include "ircparser.h"
#include "servercommandmessage.h"
#include "serverinfowidget.h"
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerCLEntry::IrcServerCLEntry (IrcServerHandler *handler,
			IrcAccount *account)
	: EntryBase (account)
	, ISH_ (handler)
	{
		QAction *showChannels = new QAction (tr ("Channels list"), this);
		connect (showChannels,
				SIGNAL (triggered ()),
				ISH_,
				SLOT (showChannels ()));
		Actions_ << showChannels;
	}

	IrcServerHandler* IrcServerCLEntry::GetIrcServerHandler () const
	{
		return ISH_;
	}

	IrcAccount* IrcServerCLEntry::GetIrcAccount () const
	{
		return Account_;
	}

	IAccount* IrcServerCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features IrcServerCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType IrcServerCLEntry::GetEntryType () const
	{
		return EntryType::MUC;
	}

	QString IrcServerCLEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + "_" + ISH_->GetServerID ();
	}

	QString IrcServerCLEntry::GetEntryName () const
	{
		return ISH_->GetServerID ();
	}

	void IrcServerCLEntry::SetEntryName (const QString&)
	{
	}

	QStringList IrcServerCLEntry::Groups () const
	{
		return { tr ("Servers") };
	}

	void IrcServerCLEntry::SetGroups (const QStringList&)
	{
	}

	QStringList IrcServerCLEntry::Variants () const
	{
		return { "" };
	}

	IMessage* IrcServerCLEntry::CreateMessage (IMessage::Type,
			const QString& variant, const QString& body)
	{
		if (variant.isEmpty ())
			return new ServerCommandMessage (body, this);
		else
			return nullptr;
	}

	IMUCEntry::MUCFeatures IrcServerCLEntry::GetMUCFeatures () const
	{
		return {};
	}

	QString IrcServerCLEntry::GetMUCSubject () const
	{
		return {};
	}

	bool IrcServerCLEntry::CanChangeSubject () const
	{
		return false;
	}

	void IrcServerCLEntry::SetMUCSubject (const QString&)
	{
	}

	QList<QObject*> IrcServerCLEntry::GetParticipants ()
	{
		return {};
	}

	bool IrcServerCLEntry::IsAutojoined () const
	{
		return false;
	}

	void IrcServerCLEntry::Join ()
	{
		ISH_->GetParser ()->NickCommand (QStringList () << ISH_->GetNickName ());
	}

	void IrcServerCLEntry::Leave (const QString&)
	{
		ISH_->SendQuit ();
	}

	QString IrcServerCLEntry::GetNick () const
	{
		return ISH_->GetNickName ();
	}

	void IrcServerCLEntry::SetNick (const QString& nick)
	{
		ISH_->SetNickName (nick);
	}

	QString IrcServerCLEntry::GetGroupName () const
	{
		return QString ();
	}

	QString IrcServerCLEntry::GetRealID (QObject*) const
	{
		return QString ();
	}

	QVariantMap IrcServerCLEntry::GetIdentifyingData () const
	{
		const auto& name = QStringLiteral ("%1 on %2:%3")
				.arg (ISH_->GetNickName (),
						ISH_->GetServerOptions ().ServerName_)
				.arg (ISH_->GetServerOptions ().ServerPort_);
		return
		{
			{ Lits::HumanReadableName, name },
			{ Lits::AccountID, ISH_->GetAccount ()->GetAccountID () },
			{ Lits::Nickname, ISH_->GetNickName () },
			{ Lits::Server, ISH_->GetServerOptions ().ServerName_ },
			{ Lits::Port, ISH_->GetServerOptions ().ServerPort_ },
			{ Lits::Encoding, ISH_->GetServerOptions ().ServerEncoding_ },
			{ Lits::SSL, ISH_->GetServerOptions ().SSL_ },
		};
	}

	void IrcServerCLEntry::InviteToMUC (const QString&, const QString&)
	{
	}

	QWidget* IrcServerCLEntry::GetConfigurationWidget ()
	{
		return new ServerInfoWidget (this);
	}

	void IrcServerCLEntry::AcceptConfiguration (QWidget*)
	{
		// there is nothing to implement
	}

	QMap<QString, QString> IrcServerCLEntry::GetISupport () const
	{
		return ISH_->GetISupport ();
	}

}
}
}
