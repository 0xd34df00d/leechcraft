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

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	class FilesModel;

	class TagsFetchManager : public QObject
	{
		Q_OBJECT

		FilesModel * const FilesModel_;

		int FetchedTags_;
		const int TotalTags_;
	public:
		TagsFetchManager (const QStringList&, Media::ITagsFetcher*, FilesModel*, QObject*);
	private slots:
		void handleTagsFetched (const QString&, const Media::AudioInfo&);
	signals:
		void tagsFetchProgress (int done, int total, QObject *thisObj);
		void tagsFetched (const QString&);
		void finished (bool);
	};
}
}
}
