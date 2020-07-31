/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARTICIPANTENTRY_H

#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;

	class IrcParticipantEntry : public EntryBase
	{
		Q_OBJECT
	protected:
		QString Nick_;
		QString UserName_;
		QString HostName_;
		QString RealName_;
		QString ServerID_;
		bool IsPrivateChat_;
	public:
		IrcParticipantEntry (const QString&, IrcAccount* = 0);

		IAccount* GetParentAccount () const;

		ICLEntry::Features GetEntryFeatures () const;
		ICLEntry::EntryType GetEntryType () const;

		QString GetEntryName () const;
		void SetEntryName (const QString&);

		virtual QString GetEntryID () const = 0;

		QStringList Variants () const;

		QString GetUserName () const;
		void SetUserName (const QString& user);

		QString GetHostName () const;
		void SetHostName (const QString& host);

		QString GetRealName () const;
		void SetRealName (const QString& realName);

		QString GetServerID () const;

		bool IsPrivateChat () const;
		void SetPrivateChat (bool isPrivate);
	private slots:
		void handleClosePrivate ();
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARTICIPANTENTRY_H
