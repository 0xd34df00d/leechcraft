/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppIq.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPBobIq : public QXmppIq
	{
		QByteArray Data_;
		QString Cid_;
		QString MimeType_;
		int MaxAge_;
	public:
		XMPPBobIq (Type = QXmppIq::Get);

		QByteArray GetData () const;
		void SetData (const QByteArray&);

		QString GetCid () const;
		void SetCid (const QString&);

		QString GetMimeType () const;
		void SetMimeType (const QString&);

		int GetMaxAge () const;
		void SetMaxAge (int);

		static bool IsBobIq (const QDomElement&);
	protected:
		void parseElementFromChild (const QDomElement&);
		void toXmlElementFromChild (QXmlStreamWriter*) const;
	};
}
}
}
