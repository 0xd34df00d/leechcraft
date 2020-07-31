/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppIq.h>
#include <QXmppDataForm.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPCaptchaIq : public QXmppIq
	{
		QXmppDataForm DataForm_;
	public:
		XMPPCaptchaIq (Type = QXmppIq::Get);

		QXmppDataForm GetDataForm () const;
		void SetDataForm (const QXmppDataForm&);
	protected:
		void toXmlElementFromChild (QXmlStreamWriter*) const;
	};
}
}
}
