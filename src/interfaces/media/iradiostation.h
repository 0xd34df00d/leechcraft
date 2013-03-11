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

#include <memory>
#include <QtPlugin>

class QUrl;

namespace Media
{
	struct AudioInfo;

	class Q_DECL_EXPORT IRadioStation
	{
	public:
		virtual ~IRadioStation () {}

		virtual QObject* GetObject () = 0;

		virtual void RequestNewStream () = 0;

		virtual QString GetRadioName () const = 0;
	protected:
		virtual void gotNewStream (const QUrl&, const AudioInfo&) = 0;

		virtual void gotPlaylist (const QString&, const QString&) = 0;

		virtual void gotError (const QString&) = 0;
	};

	typedef std::shared_ptr<IRadioStation> IRadioStation_ptr;
}

Q_DECLARE_INTERFACE (Media::IRadioStation, "org.LeechCraft.Media.IRadioStation/1.0");
