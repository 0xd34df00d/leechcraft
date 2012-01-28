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

#include "useravatarmetadata.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDomElement>
#include <QXmppElement.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsAvatarMetadataNode = "urn:xmpp:avatar:metadata";

	UserAvatarMetadata::UserAvatarMetadata (const QImage& image)
	: Width_ (image.width ())
	, Height_ (image.height ())
	, Type_ ("image/png")
	{
		QByteArray ba;
		{
			QBuffer buffer (&ba);
			buffer.open (QIODevice::WriteOnly);
			image.save (&buffer, "PNG", 0);
		}

		ID_ = QCryptographicHash::hash (ba, QCryptographicHash::Sha1).toHex ();

		Size_ = ba.size ();
	}

	QString UserAvatarMetadata::GetNodeString ()
	{
		return NsAvatarMetadataNode;
	}

	QXmppElement UserAvatarMetadata::ToXML () const
	{
		QXmppElement item;
		item.setTagName ("item");
		item.setAttribute ("id", ID_);

		QXmppElement md;
		md.setTagName ("metadata");
		md.setAttribute ("xmlns", NsAvatarMetadataNode);

		if (!IsNull ())
		{
			QXmppElement info;
			info.setTagName ("info");
			info.setAttribute ("bytes", QString::number (Size_));
			info.setAttribute ("width", QString::number (Width_));
			info.setAttribute ("height", QString::number (Height_));
			info.setAttribute ("type", Type_);
			info.setAttribute ("id", ID_);

			if (URL_.isValid ())
				info.setAttribute ("url", URL_.toEncoded ());

			md.appendChild (info);
		}

		item.appendChild (md);

		return item;
	}

	void UserAvatarMetadata::Parse (const QDomElement& elem)
	{
		const QDomElement& md = elem.firstChildElement ("metadata");
		const QDomElement& info = md.firstChildElement ("info");

		if (info.isNull ())
		{
			Size_ = 0;
			Width_ = 0;
			Height_ = 0;
			Type_ = QString ();
			ID_ = QByteArray ();
			URL_ = QUrl ();
		}
		else
		{
			Size_ = info.attribute ("bytes").toInt ();
			Width_ = info.attribute ("width").toInt ();
			Height_ = info.attribute ("height").toInt ();
			Type_ = info.attribute ("type");
			ID_ = info.attribute ("id").toLatin1 ();
			URL_ = QUrl::fromEncoded (info.attribute ("url").toAscii ());
		}
	}

	QString UserAvatarMetadata::Node () const
	{
		return NsAvatarMetadataNode;
	}

	PEPEventBase* UserAvatarMetadata::Clone () const
	{
		return new UserAvatarMetadata (*this);
	}

	bool UserAvatarMetadata::IsNull () const
	{
		return !(Size_ > 0 && Width_ > 0 && Height_ > 0);
	}

	QUrl UserAvatarMetadata::GetURL () const
	{
		return URL_;
	}

	QByteArray UserAvatarMetadata::GetID () const
	{
		return ID_;
	}
}
}
}
