/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "winwarndialog.h"
#include <QMessageBox>
#include "xmlsettingsmanager.h"

namespace LC
{
	WinWarnDialog::WinWarnDialog ()
	{
		run ();
	}
	
	void WinWarnDialog::run ()
	{
		deleteLater ();
		if (XmlSettingsManager::Instance ()->
					Property ("Win32UserAgreed", false).toBool ())
			return;
		
		const QString& text =
			tr ("Seems like you are running Microsoft Windows.\n\nLeechCraft "
				"for Windows is quite experimental and unstable, some builds "
				"may use unstable versions of third-party libraries, and, "
				"moreover, LeechCraft is known to have more bugs and less "
				"features on Windows than on other, saner operating systems. "
				"That's because we have not so much developers running Windows, "
				"and most of the users use other OSes. Sorry for that, "
				"we hope you'll understand us.\n\nIf you have read and really "
				"understood all these letters, click Cancel button in this "
				"dialog so that it won't pop up again next time you run LeechCraft.");

		if (QMessageBox::warning (0,
					"LeechCraft",
					text,
					QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
			XmlSettingsManager::Instance ()->setProperty ("Win32UserAgreed", true);
	}
}
