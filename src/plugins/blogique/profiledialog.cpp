/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "profiledialog.h"
#include <interfaces/blogique/iaccount.h>
#include "interfaces/blogique/iprofile.h"
#include "interfaces/blogique/iprofilewidget.h"
#include <QtDebug>
#include <QGridLayout>

namespace LeechCraft
{
namespace Blogique
{
	ProfileDialog::ProfileDialog (IAccount *acc, QWidget *parent)
	: QDialog (parent)
	, Account_ (acc)
	, ProfileWidget_ (0)
	{
		Ui_.setupUi (this);

		QObject *profileObj = acc->GetProfile ();
		IProfile *profile = qobject_cast<LeechCraft::Blogique::IProfile*> (profileObj);
		if (profile)
		{
			connect (profileObj,
					SIGNAL (profileUpdated ()),
					this,
					SLOT (handleProfileUpdated ()));
			QWidget* w = profile->GetProfileWidget ();
			ProfileWidget_ = qobject_cast<LeechCraft::Blogique::IProfileWidget*> (w);
			if (ProfileWidget_)
			{
				Ui_.gridLayout_2->addWidget (w);
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "widget"
						<< w
						<< "doesn't implement IProfileWidget";
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "account profile"
					<< profileObj
					<< "doesn't implement IProfile";
	}

	void ProfileDialog::handleProfileUpdated ()
	{
		if (ProfileWidget_)
			ProfileWidget_->updateProfile ();
	}
}
}

