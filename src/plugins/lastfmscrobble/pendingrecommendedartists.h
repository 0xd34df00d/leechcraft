/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "basesimilarartists.h"

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class Authenticator;

	class PendingRecommendedArtists : public BaseSimilarArtists
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;
	public:
		PendingRecommendedArtists (Authenticator*, QNetworkAccessManager*, int, QObject* = nullptr);
	private:
		void HandleData (const QByteArray&);
	private slots:
		void request ();
	};
}
}
