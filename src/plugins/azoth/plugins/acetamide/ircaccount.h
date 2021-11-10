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
#include <interfaces/azoth/ihaveconsole.h>
#include <interfaces/azoth/isupportbookmarks.h>
#include <interfaces/azoth/icanhavesslerrors.h>
#include "ircprotocol.h"
#include "localtypes.h"

namespace LC::Azoth
{
	class IProtocol;
}

namespace LC::Azoth::Acetamide
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
		State IrcAccountState_ = SOffline;

		std::shared_ptr<ClientConnection> ClientConnection_;
		bool IsFirstStart_ = true;
		QList<IrcBookmark> ActiveChannels_;
	public:
		IrcAccount (QString, QObject*);
		~IrcAccount () override;

		QObject* GetQObject () override;
		IrcProtocol* GetParentProtocol () const override;
		AccountFeatures GetAccountFeatures () const override;
		QList<QObject*> GetCLEntries () override;

		QString GetAccountName () const override;
		QString GetOurNick () const override;
		QString GetUserName () const;
		QString GetRealName () const;
		QStringList GetNickNames () const;

		std::shared_ptr<ClientConnection> GetClientConnection () const;

		void RenameAccount (const QString&) override;

		QByteArray GetAccountID () const override;
		void SetAccountID (const QString&);

		QList<QAction*> GetActions () const override;

		void OpenConfigurationDialog () override;
		void FillSettings (IrcAccountConfigurationWidget*);

		void JoinServer (ServerOptions, ChannelOptions, bool = false);

		void SetBookmarks (const QList<IrcBookmark>&);
		QList<IrcBookmark> GetBookmarks () const;

		QWidget* GetMUCBookmarkEditorWidget () override;
		QVariantList GetBookmarkedMUCs () const override;
		void SetBookmarkedMUCs (const QVariantList&) override;

		EntryStatus GetState () const override;
		void ChangeState (const EntryStatus&) override;
		void SetState (const EntryStatus& status);
		void Authorize (QObject*) override;
		void DenyAuth (QObject*) override;
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&) override;
		void RemoveEntry (QObject*) override;
		QObject* GetTransferManager () const override;

		PacketFormat GetPacketFormat () const override;
		void SetConsoleEnabled (bool) override;
		QByteArray Serialize () const;
		static IrcAccount* Deserialize (const QByteArray&, QObject*);
	private:
		void SaveActiveChannels ();
	private slots:
		void joinFromBookmarks ();
	signals:
		void gotCLItems (const QList<QObject*>&) override;
		void removedCLItems (const QList<QObject*>&) override;
		void accountRenamed (const QString&) override;
		void authorizationRequested (QObject*, const QString&) override;
		void itemSubscribed (QObject*, const QString&) override;
		void itemUnsubscribed (QObject*, const QString&) override;
		void itemUnsubscribed (const QString&, const QString&) override;
		void itemCancelledSubscription (QObject*, const QString&) override;
		void itemGrantedSubscription (QObject*, const QString&) override;
		void statusChanged (const EntryStatus&) override;
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&) override;

		void accountSettingsChanged ();

		void gotConsolePacket (const QByteArray&, IHaveConsole::PacketDirection, const QString&) override;

		void bookmarksChanged () override;

		void sslErrors (const QList<QSslError>&, const ICanHaveSslErrors::ISslErrorsReaction_ptr&) override;
	};

	using IrcAccount_ptr = std::shared_ptr<IrcAccount>;
}
