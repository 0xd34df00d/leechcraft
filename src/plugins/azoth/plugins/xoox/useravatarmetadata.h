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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_USERAVATARMETADATA_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_USERAVATARMETADATA_H
#include <QImage>
#include <QUrl>
#include "pepeventbase.h"

namespace LeechCraft
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

#endif
