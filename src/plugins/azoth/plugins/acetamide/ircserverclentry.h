/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
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
		IrcAccount* GetIrcAccount () const;

		IAccount* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type, const QString&,
				const QString&);

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		bool CanChangeSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		bool IsAutojoined () const;
		void Join ();
		void Leave (const QString& msg = QString ());
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetGroupName () const;
		QString GetRealID (QObject*) const;
		QVariantMap GetIdentifyingData () const;
		void InviteToMUC (const QString&, const QString&);

		// IConfigurableMUC
		QWidget* GetConfigurationWidget ();
		void AcceptConfiguration (QWidget*);
		QMap<QString, QString> GetISupport () const;
	signals:
		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);
		void nicknameConflict (const QString&);
		void beenKicked (const QString&);
		void beenBanned (const QString&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H
