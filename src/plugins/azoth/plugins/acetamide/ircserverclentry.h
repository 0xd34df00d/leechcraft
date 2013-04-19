/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H

#include <QObject>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "entrybase.h"

namespace LeechCraft
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
		Q_INTERFACES (LeechCraft::Azoth::IMUCEntry
				LeechCraft::Azoth::IConfigurableMUC)

		IrcServerHandler *ISH_;
		IrcAccount *Account_;
	public:
		IrcServerCLEntry (IrcServerHandler*, IrcAccount*);

		IrcServerHandler* GetIrcServerHandler () const;
		IrcAccount* GetIrcAccount () const;

		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType, const QString&,
				const QString&);
		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
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
