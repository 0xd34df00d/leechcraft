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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERLOCATION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERLOCATION_H
#include <QString>
#include <interfaces/azoth/isupportgeolocation.h>
#include "pepeventbase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class UserLocation : public PEPEventBase
	{
		GeolocationInfo_t Info_;
	public:
		static QString GetNodeString ();
		
		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;
		
		PEPEventBase* Clone () const;
		
		GeolocationInfo_t GetInfo () const;
		void SetInfo (const GeolocationInfo_t&);
	};
}
}
}

#endif
