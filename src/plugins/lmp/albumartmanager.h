/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QDir>
#include <QFutureInterface>
#include <interfaces/media/ialbumartprovider.h>
#include "interfaces/lmp/collectiontypes.h"

namespace LC::LMP
{
	class LocalCollection;

	class AlbumArtManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::AlbumArtManager)

		LocalCollection& Collection_;

		QDir AADir_;

		struct TaskQueue
		{
			Media::AlbumInfo Info_;
			QFutureInterface<QList<QImage>> Promise_;
		};
		QList<TaskQueue> Queue_;
	public:
		explicit AlbumArtManager (LocalCollection&, QObject*);

		[[nodiscard]] QFuture<QList<QImage>> CheckAlbumArt (const QString& artist, const QString& album);
		void SetAlbumArt (int id, const QString& artist, const QString& album, const QImage&);
	private:
		void CheckNewArtists (const Collection::Artists_t&);

		void HandleGotUrls (const TaskQueue&, const QList<QUrl>&);

		void ScheduleRotateQueue ();
		void RotateQueue ();

		void HandleCoversPath (const QString&);
	};
}
