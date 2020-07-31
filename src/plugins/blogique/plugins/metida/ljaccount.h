/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QSet>
#include <interfaces/blogique/iaccount.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "profiletypes.h"
#include "ljfriendentry.h"
#include "entryoptions.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJFriendEntry;
	class LJAccountConfigurationWidget;
	class LJBloggingPlatform;
	class LJXmlRPC;
	class LJProfile;

	struct LJEventProperties
	{
		QString CurrentLocation_;
		QString CurrentMood_;
		int CurrentMoodId_ = -1;
		QString CurrentMusic_;
		bool ShowInFriendsPage_ = true;
		bool AutoFormat_ = true;
		AdultContent AdultContent_ = AdultContent::WithoutAdultContent;
		CommentsManagement CommentsManagement_ = CommentsManagement::EnableComments;
		CommentsManagement ScreeningComments_ = CommentsManagement::Default;
		QString PostAvatar_;
		bool EntryVisibility_ = true;
		bool UsedRTE_ = true;
		bool NotifyByEmail_ = true;
		QStringList LikeButtons_;
		QUrl RepostUrl_;
		bool IsRepost_ = false;
	};

	struct LJEvent
	{
		//for posting
		QString Event_;
		QString Subject_;
		Access Security_ = Access::Public;
		quint32 AllowMask_ = 0;
		QDateTime DateTime_;
		QStringList Tags_;
		QString TimeZone_;
		QString UseJournal_;
		LJEventProperties Props_;

		// for getting
		qlonglong ItemID_ = -1;
		qlonglong DItemID_ = -1;
		uint ANum_ = 0;
		QUrl Url_;
	};

	class LJAccount : public QObject
					, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IAccount)

		LJBloggingPlatform * const ParentBloggingPlatform_;
		const ICoreProxy_ptr Proxy_;

		LJXmlRPC *LJXmlRpc_;
		QString Name_;
		QString Login_;
		bool IsValid_ = false;
		std::shared_ptr<LJProfile> LJProfile_;

		QAction *LoadLastEvents_;
		QAction *LoadChangedEvents_;

		enum class LastUpdateType
		{
			NoType,
			LastEntries,
			ChangedEntries
		};
		LastUpdateType LastUpdateType_ = LastUpdateType::LastEntries;

	public:
		LJAccount (const QString& name, const ICoreProxy_ptr& proxy, QObject *parent = nullptr);

		QObject* GetQObject () override;
		QObject* GetParentBloggingPlatform () const override;
		QString GetAccountName () const override;
		QString GetOurLogin () const override;
		void RenameAccount (const QString& name) override;
		QByteArray GetAccountID () const override;
		void OpenConfigurationDialog () override;
		bool IsValid () const override;

		QString GetPassword () const;

		QObject* GetProfile () override;

		void GetEntriesByDate (const QDate& date) override;
		void GetEntriesWithFilter (const Filter& filter) override;
		void RemoveEntry (const Entry& entry) override;
		void UpdateEntry (const Entry& entry) override;

		void RequestLastEntries (int count) override;
		void RequestStatistics () override;
		void RequestTags () override;

		void RequestInbox ();

		void RequestRecentComments () override;
		void AddComment (const CommentEntry& comment) override;
		void DeleteComment (qint64 id, bool deleteThread = false) override;

		QList<QAction*> GetUpdateActions () const override;

		void FillSettings (LJAccountConfigurationWidget *widget);

		QByteArray Serialize () const;
		static LJAccount* Deserialize (const QByteArray& data, const ICoreProxy_ptr& proxy, QObject *parent);

		void Validate ();
		void Init ();

		void AddFriends (const QList<LJFriendEntry_ptr>& friends);
		void AddNewFriend (const QString& username,
				const QString& bgcolor, const QString& fgcolor, uint groupMask);
		void DeleteFriend (const QString& username);

		void AddGroup (const QString& name, bool isPublic, int id);
		void DeleteGroup (int id);

		void SetMessagesAsRead (const QList<int>& ids);
		void SendMessage (const QStringList& addresses, const QString& subject,
				const QString& text);
	private:
		void CallLastUpdateMethod ();

	public slots:
		void handleValidatingFinished (bool success);
		void updateProfile () override;

		void submit (const Entry& event) override;
		void preview (const Entry& event) override;

		void handleEventPosted (const QList<LJEvent>& entries);
		void handleEventUpdated (const QList<LJEvent>& entries);
		void handleEventRemoved (int id);

		void handleGotFilteredEvents (const QList<LJEvent>& events);
		void handleGettingFilteredEventsFinished ();
		void handleGotEvents (const QList<LJEvent>& events);

		void handleLoadLastEvents ();
		void handleLoadChangedEvents ();

		void handleUnreadMessagesIds (const QList<int>& ids);
		void handleMessagesRead ();
		void handleMessageSent ();

		void handleGotRecentComments (const QList<LJCommentEntry>& comments);
		void handleCommentDeleted (const QList<qint64>& ids);
		void handleCommentSent (const QUrl& url);

	signals:
		void accountRenamed (const QString& newName) override;
		void accountSettingsChanged ();
		void accountValidated (bool validated);

		void requestEntriesBegin () override;

		void entryPosted (const QList<Entry>& entries) override;
		void entryUpdated (const QList<Entry>& entries) override;
		void entryRemoved (int itemId) override;

		void gotError(int errorCode, const QString& errorString,
				const QString& localizedErrorString = QString ()) override;

		void gotFilteredEntries (const QList<Entry>& entries) override;
		void gettingFilteredEntriesFinished () override;

		void gotEntries (const QList<Entry>& entries) override;

		void gotBlogStatistics (const QMap<QDate, int>& statistics) override;
		void tagsUpdated (const QHash<QString, int>& tags) override;

		void gotRecentComments (const QList<CommentEntry>& comments) override;
		void commentsDeleted (const QList<qint64>& comments) override;
	};
}
}
}
