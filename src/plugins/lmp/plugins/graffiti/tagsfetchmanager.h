/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace Media
{
	class ITagsFetcher;
	struct AudioInfo;
}

namespace LC::LMP::Graffiti
{
	class FilesModel;

	class TagsFetchManager : public QObject
	{
		Q_OBJECT

		FilesModel * const FilesModel_;

		int FetchedTags_ = 0;
		const int TotalTags_;
	public:
		TagsFetchManager (const QStringList&, Media::ITagsFetcher*, FilesModel*, QObject*);
	signals:
		void tagsFetchProgress (int done, int total, QObject *thisObj);
		void tagsFetched (const QString&);
		void finished ();
	};
}
