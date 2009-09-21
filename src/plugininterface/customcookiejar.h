/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGININTERFACE_CUSTOMCOOKIEJAR_H
#define PLUGININTERFACE_CUSTOMCOOKIEJAR_H
#include <QNetworkCookieJar>
#include <QByteArray>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class PLUGININTERFACE_API CustomCookieJar : public QNetworkCookieJar
		{
			Q_OBJECT

			bool FilterTrackingCookies_;
		public:
			CustomCookieJar (QObject* = 0);
			virtual ~CustomCookieJar ();

			void SetFilterTrackingCookies (bool);
			QByteArray Save () const;
			void Load (const QByteArray&);

			using QNetworkCookieJar::allCookies;
			using QNetworkCookieJar::setAllCookies;
		};
	};
};

#endif

