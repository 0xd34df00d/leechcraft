/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authclosehandler.h"
#include <QMessageBox>
#include <util/svcauth/vkauthmanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace TouchStreams
{
	AuthCloseHandler::AuthCloseHandler (Util::SvcAuth::VkAuthManager *manager, QObject *parent)
	: QObject { parent }
	, Manager_ { manager }
	{
		connect (Manager_,
				SIGNAL (authCanceled ()),
				this,
				SLOT (handleAuthCanceled ()));
	}

	void AuthCloseHandler::handleAuthCanceled ()
	{
		const auto res = QMessageBox::question (nullptr,
				"LeechCraft TouchStreams",
				tr ("Shall TouchStreams ask for VKontakte authentication next time?<br/><br/>"
					"You can always reauthenticate later."),
				QMessageBox::Yes | QMessageBox::No);
		if (res != QMessageBox::No)
			return;

		Manager_->SetSilentMode (true);
		XmlSettingsManager::Instance ().setProperty ("AuthSilentMode", true);
	}
}
}
