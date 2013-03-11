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

#include "audiostructs.h"

namespace Media
{
	struct ReleaseTrackInfo
	{
		int Number_;
		QString Name_;
		int Length_;
	};

	struct ReleaseInfo
	{
		QString ID_;

		QString Name_;
		int Year_;

		enum class Type
		{
			Standard,
			EP,
			Single,
			Other
		} Type_;

		QList<QList<ReleaseTrackInfo>> TrackInfos_;
	};

	class Q_DECL_EXPORT IPendingDisco
	{
	public:
		virtual ~IPendingDisco () {}

		virtual QObject* GetObject () = 0;

		virtual QList<ReleaseInfo> GetReleases () const = 0;
	protected:
		virtual void ready () = 0;
		virtual void error (const QString&) = 0;
	};

	class Q_DECL_EXPORT IDiscographyProvider
	{
	public:
		virtual ~IDiscographyProvider () {}

		virtual QString GetServiceName () const = 0;

		virtual IPendingDisco* GetDiscography (const QString& artist) = 0;

		virtual IPendingDisco* GetReleaseInfo (const QString& artist, const QString& release) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingDisco, "org.LeechCraft.Media.IPendingDisco/1.0");
Q_DECLARE_INTERFACE (Media::IDiscographyProvider, "org.LeechCraft.Media.IDiscographyProvider/1.0");
