/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGININTERFACE_SYNCOPS_H
#define PLUGININTERFACE_SYNCOPS_H
#include <QDataStream>
#include <QByteArray>
#include <interfaces/isyncable.h>
#include "piconfig.h"

namespace LeechCraft
{
	namespace Sync
	{
		PLUGININTERFACE_API bool operator== (const Payload&, const Payload&);

		PLUGININTERFACE_API QDataStream& operator<< (QDataStream&, const Payload&);
		PLUGININTERFACE_API QDataStream& operator>> (QDataStream&, Payload&);
		PLUGININTERFACE_API QByteArray Serialize (const Payload&);
		PLUGININTERFACE_API Payload Deserialize (const QByteArray&);

		PLUGININTERFACE_API Payload CreatePayload (const QByteArray&);
	}
}

#endif
