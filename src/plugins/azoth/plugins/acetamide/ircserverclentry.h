/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "entrybase.h"

namespace LC::Azoth::Acetamide
{
	class IrcServerHandler;
	class IrcAccount;

	class IrcServerCLEntry : public EntryBase
						   , public IMUCEntry
						   , public IConfigurableMUC
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCEntry
				LC::Azoth::IConfigurableMUC)

		IrcServerHandler *ISH_;
	public:
		IrcServerCLEntry (IrcServerHandler*, IrcAccount*);

		IrcServerHandler* GetIrcServerHandler () const;

		IAccount* GetParentAccount () const override;
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;
		QString GetEntryID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;
		QStringList Variants () const override;
		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&) override;

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const override;
		QString GetMUCSubject () const override;
		bool CanChangeSubject () const override;
		void SetMUCSubject (const QString&) override;
		QList<QObject*> GetParticipants () override;
		bool IsAutojoined () const override;
		void Join () override;
		void Leave (const QString& msg = {}) override;
		QString GetNick () const override;
		void SetNick (const QString&) override;
		QString GetGroupName () const override;
		QString GetRealID (QObject*) const override;
		QVariantMap GetIdentifyingData () const override;
		void InviteToMUC (const QString&, const QString&) override;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget () override;
		void AcceptConfiguration (QWidget*) override;
	signals:
		void gotNewParticipants (const QList<QObject*>&) override;
		void mucSubjectChanged (const QString&) override;
		void nicknameConflict (const QString&) override;
		void beenKicked (const QString&) override;
		void beenBanned (const QString&) override;
	};
}
