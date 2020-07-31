/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QXmppDataForm;
class QDialog;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPCaptchaManager;
	class XMPPBobManager;
	class FormBuilder;

	class CaptchaManager : public QObject
	{
		XMPPCaptchaManager& CaptchaManager_;
		XMPPBobManager& BobManager_;
	public:
		CaptchaManager (XMPPCaptchaManager&, XMPPBobManager&, QObject* = nullptr);
	private:
		void HandleCaptchaReceived (const QString&, const QXmppDataForm&);
	};
}
}
}
