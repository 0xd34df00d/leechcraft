 /**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <memory>
#include "ljaccount.h"
#include <QtDebug>
#include <util/xpc/passutils.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/sll/prelude.h>
#include <util/gui/util.h>
#include <interfaces/core/ientitymanager.h>
#include "ljaccountconfigurationwidget.h"
#include "ljaccountconfigurationdialog.h"
#include "ljbloggingplatform.h"
#include "ljprofile.h"
#include "ljxmlrpc.h"
#include "profilewidget.h"
#include "utils.h"
#include "xmlsettingsmanager.h"
#include "updatetypedialog.h"
#include "localstorage.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	LJAccount::LJAccount (const QString& name, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, ParentBloggingPlatform_ (qobject_cast<LJBloggingPlatform*> (parent))
	, Proxy_ (proxy)
	, LJXmlRpc_ (new LJXmlRPC (this, Proxy_, this))
	, Name_ (name)
	, LJProfile_ (std::make_shared<LJProfile> (this, proxy))
	, LoadLastEvents_ (new QAction (tr ("Last entries"), this))
	, LoadChangedEvents_ (new QAction (tr ("Changed entries"), this))
	{
		qRegisterMetaType<LJProfileData> ("LJProfileData");
#if QT_VERSION_MAJOR == 5
		qRegisterMetaTypeStreamOperators<QList<LJFriendGroup>> ("QList<LJFriendGroup>");
		qRegisterMetaTypeStreamOperators<QList<LJMood>> ("QList<LJMood>");
#endif

		connect (LJXmlRpc_,
				SIGNAL (validatingFinished (bool)),
				this,
				SLOT (handleValidatingFinished (bool)));
		connect (LJXmlRpc_,
				SIGNAL (error (int, QString, QString)),
				this,
				SIGNAL (gotError (int, QString, QString)));
		connect (LJXmlRpc_,
				SIGNAL (networkError (int, QString)),
				this,
				SIGNAL (gotError (int, QString)));
		connect (LJXmlRpc_,
				SIGNAL (profileUpdated (LJProfileData)),
				LJProfile_.get (),
				SLOT (handleProfileUpdate (LJProfileData)));
		connect (LJXmlRpc_,
				SIGNAL (gotTags (QHash<QString, int>)),
				LJProfile_.get (),
				SLOT (handleGotTags (QHash<QString, int>)));
		connect (LJProfile_.get (),
				SIGNAL (tagsUpdated (QHash<QString, int>)),
				this,
				SIGNAL (tagsUpdated (QHash<QString, int>)));
		connect (LJXmlRpc_,
				SIGNAL (eventPosted (QList<LJEvent>)),
				this,
				SLOT (handleEventPosted (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (eventRemoved (int)),
				this,
				SLOT (handleEventRemoved (int)));
		connect (LJXmlRpc_,
				SIGNAL (eventUpdated (QList<LJEvent>)),
				this,
				SLOT (handleEventUpdated (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gotFilteredEvents (QList<LJEvent>)),
				this,
				SLOT (handleGotFilteredEvents (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gettingFilteredEventsFinished ()),
				this,
				SLOT (handleGettingFilteredEventsFinished ()));
		connect (LJXmlRpc_,
				SIGNAL (gotEvents (QList<LJEvent>)),
				this,
				SLOT (handleGotEvents (QList<LJEvent>)));
		connect (LJXmlRpc_,
				SIGNAL (gotStatistics (QMap<QDate, int>)),
				this,
				SIGNAL (gotBlogStatistics (QMap<QDate, int>)));
		connect (LJXmlRpc_,
				SIGNAL (unreadMessagesIds (QList<int>)),
				this,
				SLOT (handleUnreadMessagesIds (QList<int>)));
		connect (LJXmlRpc_,
				SIGNAL (messagesRead ()),
				this,
				SLOT (handleMessagesRead ()));
		connect (LJXmlRpc_,
				SIGNAL (messageSent ()),
				this,
				SLOT (handleMessageSent ()));
		connect (LJXmlRpc_,
				SIGNAL (gotRecentComments (QList<LJCommentEntry>)),
				this,
				SLOT (handleGotRecentComments (QList<LJCommentEntry>)));
		connect (LJXmlRpc_,
				SIGNAL (commentsDeleted (QList<qint64>)),
				this,
				SLOT (handleCommentDeleted (QList<qint64>)));
		connect (LJXmlRpc_,
				SIGNAL (commentSent (QUrl)),
				this,
				SLOT (handleCommentSent (QUrl)));

		connect (LoadLastEvents_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoadLastEvents ()));
		connect (LoadChangedEvents_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleLoadChangedEvents ()));
	}

	QObject* LJAccount::GetQObject ()
	{
		return this;
	}

	QObject* LJAccount::GetParentBloggingPlatform () const
	{
		return ParentBloggingPlatform_;
	}

	QString LJAccount::GetAccountName () const
	{
		return Name_;
	}

	QString LJAccount::GetOurLogin () const
	{
		return Login_;
	}

	void LJAccount::RenameAccount (const QString&)
	{
	}

	QByteArray LJAccount::GetAccountID () const
	{
		return ParentBloggingPlatform_->GetBloggingPlatformID () + "_" +
				Login_.toUtf8();
	}

	void LJAccount::OpenConfigurationDialog ()
	{
		LJAccountConfigurationDialog dia;

		if (!Login_.isEmpty ())
			dia.ConfWidget ()->SetLogin (Login_);

		dia.ConfWidget ()->SetPassword (GetPassword ());

		if (dia.exec () == QDialog::Rejected)
			return;

		FillSettings (dia.ConfWidget ());
	}

	bool LJAccount::IsValid () const
	{
		return IsValid_;
	}

	QString LJAccount::GetPassword () const
	{
		return Util::GetPassword ("org.LeechCraft.Blogique.PassForAccount/" + GetAccountID (),
				tr ("Please enter password for the LiveJournal account %1:")
					.arg (Util::FormatName (Name_)),
				Proxy_);
	}

	QObject* LJAccount::GetProfile ()
	{
		return LJProfile_.get ();
	}

	void LJAccount::GetEntriesByDate (const QDate& date)
	{
		LJXmlRpc_->GetEventsByDate (date);
	}

	void LJAccount::GetEntriesWithFilter (const Filter& filter)
	{
		LJXmlRpc_->GetEventsWithFilter (filter);
	}

	namespace
	{
		LJEventProperties GetLJEventPropetriesFromMap (const QVariantMap& map)
		{
			LJEventProperties props;
			props.AdultContent_ = static_cast<AdultContent> (map ["adults"].toInt ());
			props.CommentsManagement_ = static_cast<CommentsManagement> (map ["comment"].toInt ());
			props.CurrentLocation_ = map ["place"].toString ();
			props.CurrentMood_ = map ["mood"].toString ();
			props.CurrentMoodId_ = map ["moodId"].toInt ();
			props.CurrentMusic_ = map ["music"].toString ();
			props.ScreeningComments_ = static_cast<CommentsManagement> (map ["hidecomment"].toInt ());
			props.PostAvatar_ = map ["avatar"].toString ();
			props.ShowInFriendsPage_ = map ["showInFriendsPage"].toBool ();
			props.LikeButtons_ = map ["likes"].toStringList ();
			return props;
		}


		QString ToLJTags (const QString& content)
		{
			static const QRegularExpression rexpOpen ("<lj-poll name=\"(.+?)\">\\s*?</lj-poll>");
			QString entry = content;
			entry.replace (rexpOpen, "<lj-poll-\\1></lj-poll-\\1>");
			return entry;
		}

		LJEvent Entry2LJEvent (const Entry& entry)
		{
			LJEvent ljEvent;
			ljEvent.UseJournal_ = entry.Target_;
			ljEvent.ItemID_ = entry.EntryId_;
			ljEvent.Event_ = ToLJTags (entry.Content_);
			ljEvent.DateTime_ = entry.Date_;
			ljEvent.Subject_ = entry.Subject_;
			ljEvent.Tags_ = entry.Tags_;
			ljEvent.AllowMask_ = entry.PostOptions_ ["allowMask"].toUInt ();
			ljEvent.Security_ = static_cast<Access> (entry.PostOptions_ ["access"].toInt ());
			ljEvent.Props_ = GetLJEventPropetriesFromMap (entry.PostOptions_);

			return ljEvent;
		}

		QVariantMap GetPostOptionsMapFromLJEvent (const LJEvent& event)
		{
			QVariantMap map;
			map ["access"] = event.Security_;
			map ["allowMask"] = event.AllowMask_;
			map ["adults"] = event.Props_.AdultContent_;
			map ["comment"] = event.Props_.CommentsManagement_;
			map ["hidecomment"] = event.Props_.ScreeningComments_;
			map ["place"] = event.Props_.CurrentLocation_;
			map ["music"] = event.Props_.CurrentMusic_;
			map ["moodId"] = event.Props_.CurrentMoodId_;
			map ["mood"] = event.Props_.CurrentMood_;
			map ["showInFriendsPage"] = event.Props_.ShowInFriendsPage_;
			map ["avatar"] = event.Props_.PostAvatar_;

			return map;
		}

		QString FromLJTags (const QString& content)
		{
			static const QRegularExpression rexpOpen ("<lj-poll-(.+?)>");
			static const QRegularExpression rexpClose ("</lj-poll-(.+?)>");
			QString entry = content;
			if (entry.contains (rexpClose))
			{
				entry.replace (rexpOpen, "<lj-poll name=\"\\1\">");
				entry.replace (rexpClose, "</lj-poll>");
			}
			else
				entry.replace (rexpOpen, "<lj-poll name=\"\\1\" />");

			return entry;
		}

		Entry LJEvent2Entry (const LJEvent& ljEvent, const QString& login)
		{
			Entry entry;
			entry.EntryId_ = ljEvent.ItemID_;
			entry.Content_ = QString ("<div>%1</div>")
					.arg (FromLJTags (ljEvent.Event_));
			entry.Date_ = ljEvent.DateTime_;
			entry.Subject_ = ljEvent.Subject_;
			entry.Tags_ = ljEvent.Tags_;
			entry.Target_ = login;
			entry.EntryUrl_ = ljEvent.Props_.IsRepost_ ?
				ljEvent.Props_.RepostUrl_ :
				ljEvent.Url_;
			entry.PostOptions_ = GetPostOptionsMapFromLJEvent (ljEvent);
			return entry;
		}
	}

	void LJAccount::RemoveEntry (const Entry& entry)
	{
		LJXmlRpc_->RemoveEvent (Entry2LJEvent (entry));
	}

	void LJAccount::UpdateEntry (const Entry& entry)
	{
		LJXmlRpc_->UpdateEvent (Entry2LJEvent (entry));
	}

	void LJAccount::RequestLastEntries (int count)
	{
		emit requestEntriesBegin ();
		LJXmlRpc_->GetLastEvents (count);
	}

	void LJAccount::RequestStatistics ()
	{
		LJXmlRpc_->RequestStatistics ();
	}

	void LJAccount::RequestTags ()
	{
		LJXmlRpc_->RequestTags ();
	}

	void LJAccount::RequestInbox ()
	{
		LJXmlRpc_->RequestLastInbox ();
	}

	void LJAccount::RequestRecentComments ()
	{
		LJXmlRpc_->RequestRecentCommments ();
	}

	void LJAccount::AddComment (const CommentEntry& comment)
	{
		LJXmlRpc_->AddComment (comment);
	}

	void LJAccount::DeleteComment (qint64 id, bool deleteThread)
	{
		LJXmlRpc_->DeleteComment (id, deleteThread);
	}

	QList<QAction*> LJAccount::GetUpdateActions () const
	{
		return { LoadLastEvents_, LoadChangedEvents_ };
	}

	void LJAccount::FillSettings (LJAccountConfigurationWidget *widget)
	{
		Login_ = widget->GetLogin ();
		const QString& pass = widget->GetPassword ();
		if (!pass.isNull ())
			Util::SavePassword (pass,
					"org.LeechCraft.Blogique.PassForAccount/" + GetAccountID (),
					Proxy_);

		emit accountSettingsChanged ();
		Validate ();
	}

	QByteArray LJAccount::Serialize () const
	{
		quint16 ver = 2;
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << ver
					<< Name_
					<< Login_
					<< IsValid_
					<< LJProfile_->GetProfileData ();
		}

		return result;
	}

	LJAccount* LJAccount::Deserialize (const QByteArray& data, const ICoreProxy_ptr& proxy, QObject *parent)
	{
		quint16 ver = 0;
		QDataStream in (data);
		in >> ver;

		if (ver > 2 ||
				ver < 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return 0;
		}

		QString name;
		in >> name;
		LJAccount *result = new LJAccount (name, proxy, parent);
		in >> result->Login_
				>> result->IsValid_;

		if (ver == 2)
		{
			LJProfileData profile;
			in >> profile;
			result->LJProfile_->handleProfileUpdate (profile);
		}

		return result;
	}

	void LJAccount::Validate ()
	{
		LJXmlRpc_->Validate (Login_, GetPassword ());
	}

	void LJAccount::Init ()
	{
		connect (this,
				SIGNAL (accountValidated (bool)),
				ParentBloggingPlatform_,
				SLOT (handleAccountValidated (bool)));

		connect (this,
				SIGNAL (accountSettingsChanged ()),
				ParentBloggingPlatform_,
				SLOT (saveAccounts ()));
	}

	void LJAccount::AddFriends (const QList<LJFriendEntry_ptr>& friends)
	{
		LJProfile_->AddFriends (friends);
	}

	void LJAccount::AddNewFriend (const QString& username,
			const QString& bgcolor, const QString& fgcolor, uint groupMask)
	{
		LJXmlRpc_->AddNewFriend (username, bgcolor, fgcolor, groupMask);
	}

	void LJAccount::DeleteFriend (const QString& username)
	{
		LJXmlRpc_->DeleteFriend (username);
	}

	void LJAccount::AddGroup (const QString& name, bool isPublic, int id)
	{
		LJXmlRpc_->AddGroup (name, isPublic, id);
	}

	void LJAccount::DeleteGroup (int id)
	{
		LJXmlRpc_->DeleteGroup (id);
	}

	void LJAccount::SetMessagesAsRead (const QList<int>& ids)
	{
		LJXmlRpc_->SetMessagesAsRead (ids);
	}

	void LJAccount::SendMessage (const QStringList& addresses, const QString& subject,
			const QString& text)
	{
		LJXmlRpc_->SendMessage (addresses, subject, text);
	}

	void LJAccount::CallLastUpdateMethod ()
	{
		switch (LastUpdateType_)
		{
		case LastUpdateType::LastEntries:
			emit requestEntriesBegin ();
			LJXmlRpc_->GetLastEvents (XmlSettingsManager::Instance ()
					.Property ("LoadEntriesToView", 20).toInt ());
			break;
		case LastUpdateType::ChangedEntries:
			emit requestEntriesBegin ();
			LJXmlRpc_->GetChangedEvents (XmlSettingsManager::Instance ()
					.Property ("ChangedDateToView",
							QDateTime ({ 1980, 1, 1 }, { 0, 0 })).toDateTime ());
			break;
		case LastUpdateType::NoType:
		default:
			break;
		}
	}

	void LJAccount::handleValidatingFinished (bool success)
	{
		IsValid_ = success;
		qDebug () << Q_FUNC_INFO
				<< "account"
				<< GetAccountID ()
				<< "validating result is"
				<< IsValid_;

		emit accountValidated (IsValid_);
		emit accountSettingsChanged ();
	}

	void LJAccount::updateProfile ()
	{
		LJXmlRpc_->UpdateProfileInfo ();
	}

	void LJAccount::submit (const Entry& entry)
	{
		LJEventProperties props;
		const QVariantMap& postOptions = entry.PostOptions_;

		LJEvent ljEvent = Entry2LJEvent (entry);

		Access access = static_cast<Access> (postOptions.value ("access").toInt ());
		ljEvent.Security_ = access < Access::MAXAccess ?
			access :
			Access::Public;
		if (access == Access::Custom)
			ljEvent.AllowMask_ = postOptions ["allowMask"].toUInt ();

		AdultContent adultContent = static_cast<AdultContent> (postOptions
				.value ("adults").toInt ());
		props.AdultContent_ = adultContent < AdultContent::MAXAdult ?
			adultContent :
			AdultContent::WithoutAdultContent;

		CommentsManagement managment, screening;
		managment = static_cast<CommentsManagement> (postOptions
				.value ("comment").toInt ());
		screening =  static_cast<CommentsManagement> (postOptions
				.value ("hidecomment").toInt ());

		props.CommentsManagement_ = managment < CommentsManagement::MAXManagment ?
			managment :
			CommentsManagement::Default;
		props.ScreeningComments_ = screening > CommentsManagement::MAXManagment &&
				screening < CommentsManagement::MAXScreening ?
			screening :
			CommentsManagement::ShowComments;

		props.CurrentLocation_ = postOptions.value ("place").toString ();
		props.CurrentMusic_ = postOptions.value ("music").toString ();

		props.CurrentMoodId_ = postOptions.value ("moodId", -1).toInt ();
		if (props.CurrentMoodId_ == -1)
			props.CurrentMood_ = postOptions.value ("mood").toString ();

		props.ShowInFriendsPage_ = postOptions.value ("showInFriendsPage").toBool ();

		props.PostAvatar_ = postOptions.value ("avatar").toString ();
		props.LikeButtons_ = postOptions.value ("likes").toStringList ();

		ljEvent.Props_ = props;
		ljEvent.Event_.append ("\n<em style=\"font-size: 0.8em;\">Posted via <a href=\"https://leechcraft.org/plugins-blogique\">LeechCraft Blogique</a>.</em>");

		static const QRegularExpression rxp
		{
			"(<lj-like.+(buttons=\"((\\w+,?)+)\"\\s?)?\\/?>).+(</lj-like>)?",
			QRegularExpression::CaseInsensitiveOption
		};
		QString buttons = QString ("<lj-like buttons=\"%1\" />")
				.arg (props.LikeButtons_.join (","));
		if (entry.Content_.contains (rxp))
			ljEvent.Event_.replace (rxp, buttons);
		else if (!ljEvent.Props_.LikeButtons_.isEmpty ())
		{
			if (XmlSettingsManager::Instance ().Property ("LikeButtonPosition", "bottom").toString () == "top")
				ljEvent.Event_.prepend (buttons);
			else
				ljEvent.Event_.append (buttons);
		}

		LJXmlRpc_->Submit (ljEvent);
	}

	void LJAccount::preview (const Entry& event)
	{
		LJXmlRpc_->Preview (Entry2LJEvent (event));
	}

	void LJAccount::handleEventPosted (const QList<LJEvent>& events)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : events)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit entryPosted (entries);
		CallLastUpdateMethod ();
	}

	void LJAccount::handleEventUpdated (const QList<LJEvent>& events)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : events)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit entryUpdated (entries);
		CallLastUpdateMethod ();
	}

	void LJAccount::handleEventRemoved (int id)
	{
		emit entryRemoved (id);
		CallLastUpdateMethod ();
	}

	void LJAccount::handleGotFilteredEvents (const QList<LJEvent>& ljEvents)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : ljEvents)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit gotFilteredEntries (entries);
	}

	void LJAccount::handleGettingFilteredEventsFinished ()
	{
		emit gettingFilteredEntriesFinished ();
	}

	void LJAccount::handleGotEvents (const QList<LJEvent>& ljEvents)
	{
		QList<Entry> entries;
		for (const auto& ljEvent : ljEvents)
			entries << LJEvent2Entry (ljEvent, Login_);

		emit gotEntries (entries);
	}

	void LJAccount::handleLoadLastEvents ()
	{
		int count = 0;
		if (XmlSettingsManager::Instance ().Property ("LoadLastAsk", true).toBool ())
		{
			UpdateTypeDialog dlg (UpdateTypeDialog::LoadType::LoadLastEvents);
			if (dlg.exec () == QDialog::Rejected)
				return;
			count = dlg.GetCount ();
		}

		LastUpdateType_ = LastUpdateType::LastEntries;
		emit requestEntriesBegin ();
		LJXmlRpc_->GetLastEvents (count);
	}

	void LJAccount::handleLoadChangedEvents ()
	{
		QDateTime dt;
		if (XmlSettingsManager::Instance ().Property ("LoadChangedAsk", true).toBool ())
		{
			UpdateTypeDialog dlg (UpdateTypeDialog::LoadType::LoadChangesEvents);
			if (dlg.exec () == QDialog::Rejected)
				return;
			dt = dlg.GetDateTime ();
		}

		LastUpdateType_ = LastUpdateType::ChangedEntries;
		emit requestEntriesBegin ();
		LJXmlRpc_->GetChangedEvents (dt);
	}

	void LJAccount::handleUnreadMessagesIds (const QList<int>& ids)
	{
		if (ids.isEmpty ())
			return;

		Entity e = Util::MakeNotification ("Blogique Metida",
				tr ("You have unread messages in account %1")
						.arg ("<em>" + GetAccountName () + "</em>"),
				Priority::Info);
		Util::NotificationActionHandler *nh =
				new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open inbox"),
				[this]
				{
					Entity urlEntity = Util::MakeEntity (QUrl ("http://livejournal.com/inbox/"),
							QString (),
							OnlyHandle | FromUserInitiated);
					Proxy_->GetEntityManager ()->HandleEntity (urlEntity);
				});
		nh->AddDependentObject (this);
		nh->AddFunction (tr ("Mark all as read"),
				[this, ids]
				{
					SetMessagesAsRead (ids);
				});
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void LJAccount::handleMessagesRead ()
	{
		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blogique Metida",
				tr ("All unread messages were marked as read"),
				Priority::Info));
	}

	void LJAccount::handleMessageSent ()
	{
		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blogique Metida",
				tr ("Message has been sent successfully"),
				Priority::Info));
	}

	namespace
	{
		CommentEntry LJCommentEntry2RecentComment (const LJCommentEntry& comment,
				const QByteArray& accountID)
		{
			CommentEntry recentComment;

			recentComment.AccountID_ = accountID;

			recentComment.EntryID_ = comment.NodeId_;
			recentComment.EntrySubject_ = comment.NodeSubject_;
			recentComment.EntryUrl_ = comment.NodeUrl_;

			recentComment.CommentSubject_ = comment.Subject_;
			recentComment.CommentText_ = comment.Text_;
			recentComment.CommentAuthor_ = comment.PosterName_;
			recentComment.CommentDateTime_ = comment.PostingDate_;
			recentComment.CommentID_ = comment.ReplyId_;
			recentComment.ParentCommentID_ = comment.ParentReplyId_;
			recentComment.CommentUrl_ = QUrl (recentComment.EntryUrl_.toString () +
					QString ("?thread=%1#t%1")
							.arg (recentComment.CommentID_));

			return recentComment;
		}
	}

	void LJAccount::handleGotRecentComments (const QList<LJCommentEntry>& comments)
	{
		if (comments.isEmpty ())
			return;

		const auto& id = GetAccountID ();
		auto recentComments = Util::Map (comments,
				[&id] (const auto& comment) { return LJCommentEntry2RecentComment (comment, id); });
		emit gotRecentComments (recentComments);
	}

	void LJAccount::handleCommentDeleted (const QList<qint64>& ids)
	{
		emit commentsDeleted (ids);
	}

	void LJAccount::handleCommentSent (const QUrl& url)
	{
		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blogique Metida",
				tr ("Reply was posted successfully:") +
						QString (" <a href=\"%1\">%1</a>\n").arg (url.toString ()),
				Priority::Info));
		LJXmlRpc_->RequestRecentCommments ();
	}

}
}
}
