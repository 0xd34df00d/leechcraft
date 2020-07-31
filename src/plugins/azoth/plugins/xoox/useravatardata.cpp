/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "useravatardata.h"
#include <QDomElement>
#include <QBuffer>
#include <QCryptographicHash>
#include <QXmppElement.h>

namespace LC
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
