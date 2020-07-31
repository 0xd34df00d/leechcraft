/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QImage>
#include <QUrl>
#include "pepeventbase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class UserAvatarMetadata : public PEPEventBase
	{
		int Size_;
		int Width_;
		int Height_;
		QString Type_;
		QByteArray ID_;
		QUrl URL_;
	public:
		UserAvatarMetadata (const QImage& = QImage ());

		static QString GetNodeString ();

		QXmppElement ToXML () const;
		void Parse (const QDomElement&);
		QString Node () const;

		PEPEventBase* Clone () const;

		bool IsNull () const;

		QUrl GetURL () const;
		QByteArray GetID () const;
	};
}
}
}
