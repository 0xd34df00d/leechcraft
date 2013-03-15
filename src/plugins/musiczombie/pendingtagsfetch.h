/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <interfaces/media/itagsfetcher.h>

class QNetworkAccessManager;

namespace LeechCraft
{
namespace Util
{
	class QueueManager;
}

namespace MusicZombie
{
	class PendingTagsFetch : public QObject
						   , public Media::IPendingTagsFetch
	{
		Q_OBJECT
		Q_INTERFACES (Media::IPendingTagsFetch)

		Util::QueueManager * const Queue_;
		QNetworkAccessManager * const NAM_;

		const QString Filename_;

		Media::AudioInfo Info_;
	public:
		PendingTagsFetch (Util::QueueManager*, QNetworkAccessManager*, const QString&);

		QObject* GetQObject ();
		Media::AudioInfo GetResult () const;
	private:
		void Request (const QByteArray&, int);
	private slots:
		void handleGotFingerprint ();
		void handleReplyFinished ();
	signals:
		void ready (const QString&, const Media::AudioInfo&);
	};
}
}
