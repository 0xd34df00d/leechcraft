/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "profiledialog.h"
#include <QtDebug>
#include <QGridLayout>
#include <interfaces/blogique/iaccount.h>
#include <interfaces/blogique/iprofile.h>
#include <interfaces/blogique/iprofilewidget.h>

namespace LC
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
		IProfile *profile = qobject_cast<IProfile*> (profileObj);
		if (!profile)
		{
			qWarning () << Q_FUNC_INFO
					<< "account profile"
					<< profileObj
					<< "doesn't implement IProfile";
			return;
		}
		connect (profileObj,
				SIGNAL (profileUpdated ()),
				this,
				SLOT (handleProfileUpdated ()));
		QWidget* w = profile->GetProfileWidget ();
		ProfileWidget_ = qobject_cast<IProfileWidget*> (w);
		if (ProfileWidget_)
			Ui_.gridLayout_2->addWidget (w);
		else
			qWarning () << Q_FUNC_INFO
					<< "widget"
					<< w
					<< "doesn't implement IProfileWidget";
	}

	void ProfileDialog::handleProfileUpdated ()
	{
		if (ProfileWidget_)
			ProfileWidget_->updateProfile ();
	}
}
}

