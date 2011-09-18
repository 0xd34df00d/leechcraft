/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "useravatardata.h"
#include <QDomElement>
#include <QBuffer>
#include <QCryptographicHash>
#include <QXmppElement.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsAvatarDataNode = "urn:xmpp:avatar:data";

	UserAvatarData::UserAvatarData (const QImage& image)
	: Img_ (image)
	{
		{
			QBuffer buffer (&Saved_);
			buffer.open (QIODevice::WriteOnly);
			image.save (&buffer, "PNG", 0);
		}

		Hash_ = QCryptographicHash::hash (Saved_, QCryptographicHash::Sha1).toHex ();
	}

	QString UserAvatarData::GetNodeString ()
	{
		return NsAvatarDataNode;
	}

	QXmppElement UserAvatarData::ToXML () const
	{
		QXmppElement item;
		item.setTagName ("item");
		item.setAttribute ("id", Hash_);

		QXmppElement data;
		data.setTagName ("data");
		data.setAttribute ("xmlns", NsAvatarDataNode);
		data.setValue (Saved_.toBase64 ());

		item.appendChild (data);

		return item;
	}

	void UserAvatarData::Parse (const QDomElement& elem)
	{
		Hash_ = elem.attribute ("id").toLatin1 ();

		const QDomElement& data = elem.firstChildElement ("data");
		Saved_ = QByteArray::fromBase64 (data.text ().toLatin1 ().trimmed ());

		Img_ = QImage::fromData (Saved_);
	}

	QString UserAvatarData::Node () const
	{
		return NsAvatarDataNode;
	}

	PEPEventBase* UserAvatarData::Clone () const
	{
		return new UserAvatarData (*this);
	}

	QImage UserAvatarData::GetImage () const
	{
		return Img_;
	}
}
}
}
