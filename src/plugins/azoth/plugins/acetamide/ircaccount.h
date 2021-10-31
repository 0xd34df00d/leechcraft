/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/ihaveconsole.h>
#include <interfaces/azoth/isupportbookmarks.h>
#include <interfaces/azoth/icanhavesslerrors.h>
#include "ircprotocol.h"
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
class IProtocol;

namespace Acetamide
{
	class ClientConnection;
	class IrcAccountConfigurationWidget;

	class IrcAccount final : public QObject
						   , public IAccount
						   , public IHaveConsole
						   , public ISupportBookmarks
						   , public ICanHaveSslErrors
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAccount
				LC::Azoth::IHaveConsole
				LC::Azoth::ISupportBookmarks
				LC::Azoth::ICanHaveSslErrors
				)

		QString AccountName_;
		IrcProtocol *ParentProtocol_;
		QByteArray AccountID_;

		QString RealName_;
		QString UserName_;
		QStringList NickNames_;
		QString DefaultServer_;
		int DefaultPort_;
		QString DefaultEncoding_;
		QString DefaultChannel_;
		State IrcAccountState_;

		std::shared_ptr<ClientConnection> ClientConnection_;
		bool IsFirstStart_ = true;
		QList<IrcBookmark> ActiveChannels_;
	public:
		IrcAccount (const QString&, QObject*);
		~IrcAccount ();
		void Init ();

		QObject* GetQObject ();
		IrcProtocol* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();

		QString GetAccountName () const;
		QString GetOurNick () const;
		QString GetUserName () const;
		QString GetRealName () const;
		QStringList GetNickNames () const;

		std::shared_ptr<ClientConnection> GetClientConnection () const;

		void RenameAccount (const QString&);

		QByteArray GetAccountID () const;
		void SetAccountID (const QString&);

		QList<QAction*> GetActions () const;

		void OpenConfigurationDialog ();
		void FillSettings (IrcAccountConfigurationWidget*);

		void JoinServer (ServerOptions, ChannelOptions, bool = false);

		void SetBookmarks (const QList<IrcBookmark>&);
		QList<IrcBookmark> GetBookmarks () const;

		QWidget* GetMUCBookmarkEditorWidget ();
		QVariantList GetBookmarkedMUCs () const;
		void SetBookmarkedMUCs (const QVariantList&);

		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void SetState (const EntryStatus& status);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		PacketFormat GetPacketFormat () const;
		void SetConsoleEnabled (bool);
		QByteArray Serialize () const;
		static IrcAccount* Deserialize (const QByteArray&, QObject*);
	private:
		void SaveActiveChannels ();
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleDestroyClient ();
		void joinFromBookmarks ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void accountRenamed (const QString&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&);

		void accountSettingsChanged ();

		void scheduleClientDestruction ();

		void gotConsolePacket (const QByteArray&, IHaveConsole::PacketDirection, const QString&);

		void bookmarksChanged ();

		void sslErrors (const QList<QSslError>&, const ICanHaveSslErrors::ISslErrorsReaction_ptr&);
	};

	typedef std::shared_ptr<IrcAccount> IrcAccount_ptr;
}
}
}
