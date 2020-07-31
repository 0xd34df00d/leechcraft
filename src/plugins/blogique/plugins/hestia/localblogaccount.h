/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/blogique/iaccount.h>

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	class AccountStorage;
	class AccountConfigurationWidget;
	class LocalBloggingPlatform;

	class LocalBlogAccount : public QObject
							, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blogique::IAccount)

		LocalBloggingPlatform *ParentBloggingPlatform_;
		QString Name_;
		bool IsValid_ = false;
		QString DatabasePath_;
		AccountStorage *AccountStorage_;

		QAction *LoadAllEvents_;
	public:
		LocalBlogAccount (const QString& name, QObject *parent = 0);

		QObject* GetQObject ();
		QObject* GetParentBloggingPlatform () const;
		QString GetAccountName () const;
		QString GetOurLogin () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		void OpenConfigurationDialog ();

		bool IsValid () const;
		QObject* GetProfile ();

		void RemoveEntry (const Entry& entry);
		void UpdateEntry (const Entry& entry);
		QList<QAction*> GetUpdateActions () const;

		void RequestLastEntries (int count = 0);
		void RequestStatistics ();
		void RequestTags ();
		void GetEntriesByDate (const QDate& date);
		void GetEntriesWithFilter (const Filter& filter);

		void RequestRecentComments ();
		void AddComment (const CommentEntry& comment);
		void DeleteComment (qint64 id, bool deleteThread = false);

		QHash<QString, int> GetTags () const;

		void FillSettings (AccountConfigurationWidget *widget);
		void Init ();

		QByteArray Serialize () const;
		static LocalBlogAccount* Deserialize (const QByteArray& data, QObject *parent);
	private:
		void Validate ();

	public slots:
		void updateProfile ();
		void submit (const Entry& event);
		void preview (const Entry& event);

		void handleLoadAllEvents ();

	signals:
		void accountRenamed (const QString& newName);
		void accountSettingsChanged ();
		void accountValidated (bool validated);

		void requestEntriesBegin ();

		void entryPosted (const QList<Entry>& entries);
		void entryRemoved (int itemId);
		void entryUpdated (const QList<Entry>& entries);

		void gotEntries (const QList<Entry>& entries);
		void gotFilteredEntries(const QList< Entry >& entries);
		void gettingFilteredEntriesFinished();
		void gotBlogStatistics (const QMap<QDate, int>& statistics);
		void tagsUpdated (const QHash<QString, int>& tags);
		void gotRecentComments (const QList<CommentEntry>& comments);
		void commentsDeleted (const QList<qint64>& comments);

		void gotError(int errorCode, const QString& errorString,
				const QString& localizedErrorString = QString ());

	};
}
}
}

