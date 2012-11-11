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

#include "winwarndialog.h"
#include <QMessageBox>
#include "xmlsettingsmanager.h"

namespace LeechCraft
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
