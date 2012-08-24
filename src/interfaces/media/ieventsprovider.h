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

#include <QDateTime>
#include <QUrl>
#include <QStringList>
#include <QtPlugin>

namespace Media
{
	struct EventInfo
	{
		QString Name_;
		QString Description_;
		QDateTime Date_;
		QUrl URL_;

		QUrl SmallImage_;
		QUrl BigImage_;

		QStringList Artists_;
		QString Headliner_;

		QStringList Tags_;

		int Attendees_;

		QString PlaceName_;
		double Latitude_;
		double Longitude_;
		QString City_;
		QString Address_;
	};

	typedef QList<EventInfo> EventInfos_t;

	class IEventsProvider
	{
	public:
		virtual ~IEventsProvider () {}

		virtual QString GetServiceName () const = 0;

		virtual void UpdateRecommendedEvents () = 0;
	protected:
		virtual void gotRecommendedEvents (const EventInfos_t&) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IEventsProvider, "org.LeechCraft.Media.IEventsProvider/1.0");
