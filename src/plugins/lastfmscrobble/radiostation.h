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

#include <memory>
#include <QObject>
#include <interfaces/media/iradiostation.h>
#include <interfaces/media/iradiostationprovider.h>
#include "lastfmheaders.h"

class QNetworkAccessManager;

namespace LeechCraft
{
namespace Lastfmscrobble
{
	class RadioTuner;

	class RadioStation : public QObject
					   , public Media::IRadioStation
	{
		Q_OBJECT
		Q_INTERFACES (Media::IRadioStation)

		std::shared_ptr<RadioTuner> Tuner_;
	public:
		struct UnsupportedType {};

		RadioStation (QNetworkAccessManager*, Media::IRadioStationProvider::Type, const QString&);

		QObject* GetObject ();
		void RequestNewStream ();
	private:
		void EmitTrack (const lastfm::Track&);
	private slots:
		void handleTitle (const QString&);
		void handleError (const QString&);
		void handleNextTrack ();
	signals:
		void gotNewStream (const QUrl&, const Media::AudioInfo&);
		void gotError (const QString&);
	};
}
}
