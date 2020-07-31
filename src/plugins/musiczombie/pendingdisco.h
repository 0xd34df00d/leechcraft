/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <QFutureInterface>
#include <interfaces/media/idiscographyprovider.h>

class QNetworkAccessManager;

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace MusicZombie
{
	class PendingDisco : public QObject
	{
		const QString Artist_;
		const QString ReleaseName_;
		const QSet<QString> Hints_;

		Util::QueueManager *Queue_;

		QNetworkAccessManager *NAM_;

		using QueryResult_t = Media::IDiscographyProvider::Result_t;
		QFutureInterface<QueryResult_t> Promise_;

		using Artist2Releases_t = QHash<QString, QSet<QString>>;
	public:
		PendingDisco (Util::QueueManager*, const QString&, const QString&,
				const QStringList&, QNetworkAccessManager*, QObject* = nullptr);

		QFuture<Media::IDiscographyProvider::Result_t> GetFuture ();
	private:
		void RequestArtist (bool);

		void HandleData (const QByteArray&, bool);
		void HandleDataNoHints (const Artist2Releases_t&);
		void HandleDataWithHints (Artist2Releases_t&);

		void HandleGotID (const QString&);

		void HandleLookupFinished (const QByteArray&);
	};
}
}
