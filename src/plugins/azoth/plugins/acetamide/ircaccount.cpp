/**********************************************************************
* LeechCraft - modular cross-platform feature rich internet client.
* Copyright (C) 2010-2011 Oleg Linkin
*
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircaccount.h"
#include <QInputDialog>
#include <QSettings>
#include <QTimer>
#include <util/sll/prelude.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iproxyobject.h>
#include "channelclentry.h"
#include "clientconnection.h"
#include "core.h"
#include "ircaccountconfigurationdialog.h"
#include "ircaccountconfigurationwidget.h"
#include "ircprotocol.h"
#include "ircserverclentry.h"
#include "xmlsettingsmanager.h"
#include "ircserverhandler.h"
#include "channelhandler.h"
#include "bookmarkeditwidget.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	IrcAccount::IrcAccount (const QString& name, QObject *parent)
	: QObject (parent)
	, AccountName_ (name)
	, ParentProtocol_ (qobject_cast<IrcProtocol*> (parent))
	, IrcAccountState_ (SOffline)
	{
		connect (this,
				SIGNAL (scheduleClientDestruction ()),
				this,
				SLOT (handleDestroyClient ()),
				Qt::QueuedConnection);
		Init ();
	}

	IrcAccount::~IrcAccount()
	{
		emit removedCLItems (GetCLEntries ());
	}

	void IrcAccount::Init ()
	{
		ClientConnection_.reset (new ClientConnection (this));

		connect (ClientConnection_.get (),
				SIGNAL (gotRosterItems (const QList<QObject*>&)),
				this,
				SLOT (handleGotRosterItems (const QList<QObject*>&)));

		connect (ClientConnection_.get (),
				SIGNAL (rosterItemRemoved (QObject*)),
				this,
				SLOT (handleEntryRemoved (QObject*)));

		connect (ClientConnection_.get (),
				SIGNAL (rosterItemsRemoved (const QList<QObject*>&)),
				this,
				SIGNAL (removedCLItems (const QList<QObject*>&)));

		connect (ClientConnection_.get (),
				SIGNAL (gotConsoleLog (QByteArray, IHaveConsole::PacketDirection, QString)),
				this,
				SIGNAL (gotConsolePacket (QByteArray, IHaveConsole::PacketDirection, QString)));
	}

	QObject* IrcAccount::GetQObject ()
	{
		return this;
	}

	QObject* IrcAccount::GetParentProtocol () const
	{
		return ParentProtocol_;
	}

	IAccount::AccountFeatures IrcAccount::GetAccountFeatures () const
	{
		return FRenamable | FMUCsSupportFileTransfers;
	}

	QList<QObject*> IrcAccount::GetCLEntries ()
	{
		return ClientConnection_ ?
				ClientConnection_->GetCLEntries () :
				QList<QObject*> ();
	}

	QString IrcAccount::GetAccountName () const
	{
		return AccountName_;
	}

	QString IrcAccount::GetOurNick () const
	{
		return "R!";
	}

	QString IrcAccount::GetUserName () const
	{
		return UserName_;
	}

	QString IrcAccount::GetRealName () const
	{
		return RealName_;
	}

	QStringList IrcAccount::GetNickNames () const
	{
		return NickNames_;
	}

	std::shared_ptr<ClientConnection> IrcAccount::GetClientConnection () const
	{
		return ClientConnection_;
	}

	void IrcAccount::RenameAccount (const QString& name)
	{
		AccountName_ = name;
		emit accountRenamed (name);
		emit accountSettingsChanged ();
	}

	QByteArray IrcAccount::GetAccountID () const
	{
		return AccountID_;
	}

	void IrcAccount::SetAccountID (const QString& id)
	{
		AccountID_ = id.toUtf8 ();
	}

	QList<QAction*> IrcAccount::GetActions () const
	{
		return QList<QAction*> ();
	}

	void IrcAccount::OpenConfigurationDialog ()
	{
		IrcAccountConfigurationDialog dia { nullptr };

		if (!RealName_.isEmpty ())
			dia.ConfWidget ()->SetRealName (RealName_);
		if (!UserName_.isEmpty ())
			dia.ConfWidget ()->SetUserName (UserName_);
		if (!NickNames_.isEmpty ())
			dia.ConfWidget ()->SetNickNames (NickNames_);
		if (!DefaultServer_.isEmpty ())
			dia.ConfWidget ()->SetDefaultServer (DefaultServer_);
		if (!DefaultPort_)
			dia.ConfWidget ()->SetDefaultPort (DefaultPort_);
		if (!DefaultEncoding_.isEmpty ())
			dia.ConfWidget ()->SetDefaultEncoding (DefaultEncoding_);
		if (!DefaultChannel_.isEmpty ())
			dia.ConfWidget ()->SetDefaultChannel (DefaultChannel_);

		if (dia.exec () == QDialog::Rejected)
			return;

		FillSettings (dia.ConfWidget ());
	}

	void IrcAccount::FillSettings (IrcAccountConfigurationWidget *widget)
	{
		State lastState = IrcAccountState_;
		if (lastState != SOffline &&
			(RealName_ != widget->GetRealName () ||
			UserName_ != widget->GetUserName () ||
			NickNames_ != widget->GetNickNames ()))
		{
			ChangeState (EntryStatus (SOffline, QString  ()));
		}

		RealName_ = widget->GetRealName ();
		UserName_ = widget->GetUserName ();
		NickNames_ = widget->GetNickNames ();
		DefaultServer_ = widget->GetDefaultServer ();
		DefaultPort_ = widget->GetDefaultPort ();
		DefaultEncoding_ = widget->GetDefaultEncoding ();
		DefaultChannel_ = widget->GetDefaultChannel ();


		if (lastState != SOffline)
			ChangeState (EntryStatus (lastState, QString ()));

		emit accountSettingsChanged ();
	}

	void IrcAccount::JoinServer (ServerOptions server,
			ChannelOptions channel, bool onlyServer)
	{
		if (server.ServerName_.isEmpty ())
			server.ServerName_ = DefaultServer_;
		if (!server.ServerPort_)
			server.ServerPort_ = DefaultPort_;
		if (server.ServerEncoding_.isEmpty ())
			server.ServerEncoding_ = DefaultEncoding_;
		if (server.ServerNickName_.isEmpty ())
			server.ServerNickName_ = NickNames_.value (0, GetOurNick ());

		if (channel.ServerName_.isEmpty ())
			channel.ServerName_ = server.ServerName_;
		if (channel.ChannelName_.isEmpty ())
			channel.ChannelName_ = DefaultChannel_;

		if (!ClientConnection_->IsServerExists (server))
		{
			ClientConnection_->JoinServer (server);
			if (!onlyServer)
				ClientConnection_->GetIrcServerHandler (server)->
						Add2ChannelsQueue (channel);
		}
		else if (!onlyServer)
			ClientConnection_->JoinChannel (server, channel);
	}

	void IrcAccount::SetBookmarks (const QList<IrcBookmark>& bookmarks)
	{
		if (!ClientConnection_)
			return;

		ClientConnection_->SetBookmarks (bookmarks);
	}

	QList<IrcBookmark> IrcAccount::GetBookmarks () const
	{
		if (!ClientConnection_)
			return QList<IrcBookmark> ();

		return ClientConnection_->GetBookmarks ();
	}

	QWidget* IrcAccount::GetMUCBookmarkEditorWidget ()
	{
		return new BookmarkEditWidget ();
	}

	QVariantList IrcAccount::GetBookmarkedMUCs () const
	{
		return Util::Map (GetBookmarks (),
				[this] (const IrcBookmark& channel) -> QVariant
				{
					const auto& name = QStringLiteral ("%1@%2 (%3)")
							.arg (channel.ChannelName_ ,
									channel.ServerName_,
									channel.NickName_);
					return QVariantMap
					{
						{ Lits::HumanReadableName, name },
						{ Lits::AccountID, GetAccountID () },
						{ Lits::Server, channel.ServerName_ },
						{ Lits::Port, channel.ServerPort_ },
						{ Lits::ServerPassword, channel.ServerPassword_ },
						{ Lits::Encoding, channel.ServerEncoding_ },
						{ Lits::Channel, channel.ChannelName_ },
						{ Lits::ChannelPassword, channel.ChannelPassword_ },
						{ Lits::Nickname, channel.NickName_ },
						{ Lits::SSL, channel.SSL_ },
						{ Lits::Autojoin, channel.AutoJoin_ },
						{ Lits::StoredName, channel.Name_ },
					};
				});
	}

	void IrcAccount::SetBookmarkedMUCs (const QVariantList& datas)
	{
		const auto& channels = Util::Map (datas,
				[] (const QVariant& var)
				{
					const auto& map = var.toMap ();

					return IrcBookmark
					{
						.Name_ = map [Lits::StoredName].toString (),
						.ServerName_ = map [Lits::Server].toString (),
						.ServerPassword_ = map [Lits::ServerPassword].toString (),
						.ServerEncoding_ = map [Lits::Encoding].toString (),
						.NickName_ = map [Lits::Nickname].toString (),
						.ChannelName_ = map [Lits::Channel].toString (),
						.ChannelPassword_ = map [Lits::ChannelPassword].toString (),
						.ServerPort_ = map [Lits::Port].toInt (),
						.SSL_ = map [Lits::SSL].toBool (),
						.AutoJoin_  = map [Lits::Autojoin].toBool (),
					};
				});

		SetBookmarks (channels);
	}

	EntryStatus IrcAccount::GetState () const
	{
		return EntryStatus (IrcAccountState_, QString ());
	}

	void IrcAccount::ChangeState (const EntryStatus& state)
	{
		if ((IrcAccountState_ == SOffline &&
				!ClientConnection_) ||
				(IrcAccountState_ == state.State_ &&
				IrcAccountState_ == SOffline))
			return;

		IProxyObject *obj = qobject_cast<IProxyObject*> (ParentProtocol_->GetProxyObject ());
		bool autoJoin = false;
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "is not an object of IProxyObject"
					<< ParentProtocol_->GetProxyObject ();
		}
		else
			autoJoin = obj->GetSettingsManager ()->
					property ("IsAutojoinAllowed").toBool ();

		EntryStatus newStatus = state;
		switch (state.State_)
		{
			case SDND:
			case SXA:
				newStatus.State_ = SAway;
				break;
			case SChat:
				newStatus.State_ = SOnline;
				break;
			default:
				break;
		}

		if (newStatus.State_ == SOffline)
		{
			if (ClientConnection_->GetServerHandlers ().count ())
				SaveActiveChannels ();
			ClientConnection_->DisconnectFromAll ();
			SetState (newStatus);
		}
		else
		{
			if (newStatus.State_ == SOnline)
			{
				if (IrcAccountState_ == SAway)
					ClientConnection_->SetAway (false, QString ());
				else
					SetState (newStatus);
			}
			else if (newStatus.State_ == SAway)
				ClientConnection_->SetAway (true, newStatus.StatusString_);

			if (autoJoin)
			{
				if (ActiveChannels_.isEmpty ())
					ActiveChannels_ << GetBookmarks ();

				if (IsFirstStart_)
					QTimer::singleShot (10000, this, SLOT (joinFromBookmarks ()));
				else
					joinFromBookmarks ();
			}
			else
				joinFromBookmarks ();
		}

		IsFirstStart_ = false;
	}

	void IrcAccount::SetState (const EntryStatus& status)
	{
		IrcAccountState_ = status.State_;
		emit statusChanged (status);
	}

	void IrcAccount::Authorize (QObject*)
	{
	}

	void IrcAccount::DenyAuth (QObject*)
	{
	}

	void IrcAccount::RequestAuth (const QString&, const QString&,
			const QString&, const QStringList&)
	{
	}

	void IrcAccount::RemoveEntry (QObject*)
	{
	}

	QObject* IrcAccount::GetTransferManager () const
	{
		return 0;
	}

	IHaveConsole::PacketFormat IrcAccount::GetPacketFormat () const
	{
		return PacketFormat::PlainText;
	}

	void IrcAccount::SetConsoleEnabled (bool enabled)
	{
		ClientConnection_->SetConsoleEnabled (enabled);
	}

	QByteArray IrcAccount::Serialize () const
	{
		quint16 version = 3;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
					<< AccountName_
					<< AccountID_
					<< RealName_
					<< UserName_
					<< NickNames_
					<< DefaultServer_
					<< DefaultPort_
					<< DefaultEncoding_
					<< DefaultChannel_;
		}

		return result;
	}

	IrcAccount* IrcAccount::Deserialize (const QByteArray& data,
			QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version < 1 || version > 3)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString name;
		in >> name;
		const auto result = new IrcAccount (name, parent);
		in >> result->AccountID_
				>> result->RealName_
				>> result->UserName_
				>> result->NickNames_;
		if (version < 3)
		{
			result->DefaultServer_ = "chat.freenode.net";
			result->DefaultPort_ = 8001;
			result->DefaultEncoding_ = "UTF-8";
			result->DefaultChannel_ = "leechcraft";
		}
		else
			in >> result->DefaultServer_
					>> result->DefaultPort_
					>> result->DefaultEncoding_
					>> result->DefaultChannel_;

		result->Init ();

		return result;
	}

	void IrcAccount::SaveActiveChannels ()
	{
		ActiveChannels_.clear ();
		for (auto ish : ClientConnection_->GetServerHandlers ())
			for (auto ich : ish->GetChannelHandlers ())
			{
				IrcBookmark bookmark;
				bookmark.ServerName_ = ish->GetServerOptions ().ServerName_;
				bookmark.ServerPort_ = ish->GetServerOptions ().ServerPort_;
				bookmark.ServerPassword_ = ish->GetServerOptions ().ServerPassword_;
				bookmark.ServerEncoding_ = ish->GetServerOptions ().ServerEncoding_;
				bookmark.NickName_ = ish->GetServerOptions ().ServerNickName_;
				bookmark.SSL_ = ish->GetServerOptions ().SSL_;
				bookmark.ChannelName_ = ich->GetChannelOptions ().ChannelName_;
				bookmark.ChannelPassword_ = ich->GetChannelOptions ().ChannelPassword_;
				bookmark.AutoJoin_ = true;
				ActiveChannels_ << bookmark;
			}
	}

	void IrcAccount::handleEntryRemoved (QObject *entry)
	{
		emit removedCLItems ({ entry });
	}

	void IrcAccount::handleGotRosterItems (const QList<QObject*>& items)
	{
		emit gotCLItems (items);
	}

	void IrcAccount::handleDestroyClient ()
	{
	}

	void IrcAccount::joinFromBookmarks ()
	{
		ServerOptions serverOpt;
		ChannelOptions channelOpt;

		for (const auto& bookmark : ActiveChannels_)
		{
			if (!bookmark.AutoJoin_)
				continue;

			serverOpt.ServerName_ = bookmark.ServerName_;
			serverOpt.ServerPort_ = bookmark.ServerPort_;
			serverOpt.ServerPassword_ = bookmark.ServerPassword_;
			serverOpt.ServerEncoding_ = bookmark.ServerEncoding_;
			serverOpt.ServerNickName_ = bookmark.NickName_;
			serverOpt.SSL_ = bookmark.SSL_;

			channelOpt.ServerName_ = bookmark.ServerName_;
			channelOpt.ChannelName_ = bookmark.ChannelName_;
			channelOpt.ChannelPassword_ = bookmark.ChannelPassword_;

			// TODO NickServ for bookmarks
			JoinServer (serverOpt, channelOpt);
		}

		ActiveChannels_.clear ();
	}
}
}
}
