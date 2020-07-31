/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERLOCATION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERLOCATION_H
#include <QString>
#include <interfaces/azoth/isupportgeolocation.h>
#include "pepeventbase.h"

namespace LC
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
		
		QXmppElement ToXML () const override;
		void Parse (const QDomElement&) override;
		QString Node () const override;
		
		PEPEventBase* Clone () const override;
		
		GeolocationInfo_t GetInfo () const;
		void SetInfo (const GeolocationInfo_t&);
	};
}
}
}

#endif
