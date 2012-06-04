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

#include <QString>
#include <QDateTime>
#include <QUrl>

namespace Media
{
	struct AlbumRelease
	{
		QString Title_;
		QString Artist_;
		QDateTime Date_;

		QUrl ThumbImage_;
		QUrl FullImage_;

		QUrl ReleaseURL_;
	};

	class IRecentReleases
	{
	public:
		virtual ~IRecentReleases () {}

		virtual void RequestRecentReleases (int number, bool withRecommends) = 0;

		virtual QString GetServiceName () const = 0;
	protected:
		virtual void gotRecentReleases (const QList<AlbumRelease>&) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IRecentReleases, "org.LeechCraft.Media.IRecentReleases/1.0");
