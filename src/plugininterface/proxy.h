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

#ifndef PLUGININTERFACE_PROXY_H
#define PLUGININTERFACE_PROXY_H
#include <QObject>
#include <QStringList>
#include <QTime>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		/*! @brief Provides some common features.
		 *
		 * Feature versions of Proxy class may include some sort of
		 * communications with MainWindow class as it was before removing of
		 * LogShower in main LeechCraft application.
		 *
		 */
		class Proxy : public QObject
		{
			Q_OBJECT

			Proxy ();
			~Proxy ();

			static Proxy *Instance_;
			QStringList Strings_;
		public:
			PLUGININTERFACE_API static Proxy *Instance ();
			PLUGININTERFACE_API void SetStrings (const QStringList&);
			PLUGININTERFACE_API QString GetApplicationName () const;
			PLUGININTERFACE_API QString GetOrganizationName () const;
			PLUGININTERFACE_API QString MakePrettySize (qint64) const;
			PLUGININTERFACE_API QString MakeTimeFromLong (ulong) const;
		};
	};
};

#endif

