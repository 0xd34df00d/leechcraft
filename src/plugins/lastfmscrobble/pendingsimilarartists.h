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
	class PendingSimilarArtists : public BaseSimilarArtists
	{
		QNetworkAccessManager *NAM_;
	public:
		PendingSimilarArtists (const QString&, int num, QNetworkAccessManager*, QObject* = nullptr);
	private slots:
		void HandleData (const QByteArray&);
	};
}
}
