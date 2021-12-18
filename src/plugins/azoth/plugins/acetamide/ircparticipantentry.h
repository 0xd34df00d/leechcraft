/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include "entrybase.h"

namespace LC::Azoth::Acetamide
{
	class IrcAccount;

	class IrcParticipantEntry : public EntryBase
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Acetamide::IrcParticipantEntry)
	protected:
		QString Nick_;
		QString UserName_;
		QString HostName_;
		QString RealName_;
		QString ServerID_;
		bool IsPrivateChat_ = false;
	public:
		explicit IrcParticipantEntry (QString, IrcAccount* = nullptr);

		IAccount* GetParentAccount () const override;

		ICLEntry::Features GetEntryFeatures () const override;
		ICLEntry::EntryType GetEntryType () const override;

		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;

		QString GetEntryID () const override = 0;

		QStringList Variants () const override;

		QString GetUserName () const;
		void SetUserName (const QString& user);

		QString GetHostName () const;
		void SetHostName (const QString& host);

		QString GetRealName () const;
		void SetRealName (const QString& realName);

		QString GetServerID () const;

		bool IsPrivateChat () const;
		void SetPrivateChat (bool isPrivate);
	};
}
