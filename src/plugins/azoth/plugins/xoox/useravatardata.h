/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERAVATARDATA_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERAVATARDATA_H
#include <QImage>
#include "pepeventbase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class UserAvatarData : public PEPEventBase
	{
		QImage Img_;
		QByteArray Saved_;
		QByteArray Hash_;
	public:
		UserAvatarData (const QImage& = QImage ());

		static QString GetNodeString ();

		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;

		PEPEventBase* Clone () const;

		QImage GetImage () const;
	};
}
}
}

#endif
