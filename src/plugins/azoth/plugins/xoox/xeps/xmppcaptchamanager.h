/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPCaptchaManager : public QXmppClientExtension
	{
		Q_OBJECT
	public:
		bool handleStanza (const QDomElement&);

		QString SendResponse (const QString& to, const QXmppDataForm& form);
	signals:
		void captchaFormReceived (const QString& from, const QXmppDataForm& form);
	};
}
}
}
