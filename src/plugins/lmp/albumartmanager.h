/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDir>
#include <interfaces/media/ialbumartprovider.h>
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	class AlbumArtManager : public QObject
	{
		Q_OBJECT

		QDir AADir_;

		struct TaskQueue
		{
			Media::AlbumInfo Info_;
			bool PreviewMode_;
		};
		QList<TaskQueue> Queue_;
		QHash<Media::AlbumInfo, int> NumRequests_;

		QHash<Media::AlbumInfo, QSize> BestSizes_;
	public:
		AlbumArtManager (QObject*);

		void CheckAlbumArt (const Collection::Artist&, Collection::Album_ptr);
		void CheckAlbumArt (const QString& artist, const QString& album, bool preview);

		void HandleGotAlbumArt (const Media::AlbumInfo&, const QList<QImage>&);
	private:
		void HandleGotUrls (const TaskQueue&, const QList<QUrl>&);
	private slots:
		void rotateQueue ();

		void handleCoversPath ();
	signals:
		void gotImages (const Media::AlbumInfo&, const QList<QImage>&);
	};
}
}
