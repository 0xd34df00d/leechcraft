/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "useravatarmetadata.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDomElement>
#include <QXmppElement.h>

namespace LC
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
			URL_ = QUrl::fromEncoded (info.attribute ("url").toLatin1 ());
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
