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
#include <interfaces/media/ialbumartprovider.h>
#include <util/threads/coro/channel.h>
#include "interfaces/lmp/collectiontypes.h"

namespace LC::LMP
{
	class LocalCollection;

	class AlbumArtManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::AlbumArtManager)

		LocalCollection& Collection_;

		QDir AADir_;
	public:
		explicit AlbumArtManager (LocalCollection&, QObject*);

		[[nodiscard]] Util::Channel_ptr<QImage> CheckAlbumArt (const QString& artist, const QString& album);
		void SetAlbumArt (int id, const QString& artist, const QString& album, const QImage&);
	private:
		Util::ContextTask<void> CheckNewArtists (Collection::Artists_t);
		void HandleCoversPath (const QString&);
	};
}
