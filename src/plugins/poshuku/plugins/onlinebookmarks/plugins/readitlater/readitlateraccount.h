/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERACCOUNT_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERACCOUNT_H

#include <QObject>
#include <QDateTime>
#include <interfaces/structures.h>
#include <interfaces/iaccount.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
class ReadItLaterAccount : public QObject
							, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::OnlineBookmarks::IAccount)

		QString Login_;
		QString Password_;

		QObject *ParentService_;

		bool IsSyncing_ = false;

		QDateTime LastUpload_;
		QDateTime LastDownload_;

		QVariantList DownloadedBookmarks_;
	public:
		explicit ReadItLaterAccount (const QString&, QObject* = nullptr);

		QObject* GetQObject () override;
		QObject* GetParentService () const override;
		QByteArray GetAccountID () const override;
		QString GetLogin () const override;

		QString GetPassword () const override;
		void SetPassword (const QString&) override;

		bool IsSyncing () const override;
		void SetSyncing (bool) override;

		QDateTime GetLastDownloadDateTime () const override;
		QDateTime GetLastUploadDateTime () const override;
		void SetLastUploadDateTime (const QDateTime&) override;
		void SetLastDownloadDateTime (const QDateTime&) override;

		void AppendDownloadedBookmarks (const QVariantList&);
		QVariantList GetBookmarksDiff (const QVariantList&);

		QByteArray Serialize () const ;
		static ReadItLaterAccount* Deserialize (const QByteArray&, QObject*);
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERACCOUNT_H
