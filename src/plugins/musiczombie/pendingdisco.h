/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/media/idiscographyprovider.h>

class QNetworkAccessManager;

namespace LeechCraft
{
namespace Util
{
	class QueueManager;
}

namespace MusicZombie
{
	class PendingDisco : public QObject
					   , public Media::IPendingDisco
	{
		Q_OBJECT
		Q_INTERFACES (Media::IPendingDisco)

		const QString ReleaseName_;

		Util::QueueManager *Queue_;

		QNetworkAccessManager *NAM_;
		QList<Media::ReleaseInfo> Releases_;
		int PendingReleases_;
	public:
		PendingDisco (Util::QueueManager*, const QString&, const QString&, QNetworkAccessManager*, QObject* = 0);

		QObject* GetObject ();

		QList<Media::ReleaseInfo> GetReleases () const;
	private:
		void DecrementPending ();
	private slots:
		void handleGotID (const QString&);
		void handleIDError ();

		void handleLookupFinished ();
		void handleLookupError ();

		void handleReleaseLookupFinished ();
		void handleReleaseLookupError ();
	signals:
		void ready ();
		void error (const QString&);
	};
}
}
