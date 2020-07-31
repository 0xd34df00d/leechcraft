/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QFutureInterface>
#include <util/sll/either.h>
#include <interfaces/media/audiostructs.h>

class QNetworkReply;

namespace LC
{
namespace Lastfmscrobble
{
	class BaseSimilarArtists : public QObject
	{
		Media::SimilarityInfos_t Similar_;

		QFutureInterface<Media::SimilarityQueryResult_t> Promise_;
	protected:
		const int NumGet_;
		int InfosWaiting_ = 0;
	public:
		BaseSimilarArtists (int, QObject* = nullptr);

		QFuture<Media::SimilarityQueryResult_t> GetFuture ();
	protected:
		void ReportError (const QString&);

		void HandleReply (QNetworkReply*, const std::optional<int>&, const std::optional<QStringList>&);
	private:
		void DecrementWaiting ();
		void HandleInfoReplyFinished (const QByteArray&, const std::optional<int>&, const std::optional<QStringList>&);
	};
}
}
