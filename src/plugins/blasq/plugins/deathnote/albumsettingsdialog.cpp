/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumsettingsdialog.h"
#include <QPushButton>
#include <QtDebug>
#include <util/gui/clearlineeditaddon.h>
#include "fotobilderaccount.h"
#include "selectgroupsdialog.h"

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	AlbumSettingsDialog::AlbumSettingsDialog (const QString& name, const QString& login,
			FotoBilderAccount *acc, QWidget *parent)
	: QDialog (parent)
	, PrivacyLevel_ (255)
	, Login_ (login)
	, Account_ (acc)
	{
		Ui_.setupUi (this);
		Ui_.Name_->setText (name);

		new Util::ClearLineEditAddon (Account_->GetProxy (), Ui_.Name_);

		connect (Ui_.Name_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (validate ()));
		validate ();
	}

	QString AlbumSettingsDialog::GetName () const
	{
		return Ui_.Name_->text ();
	}

	int AlbumSettingsDialog::GetPrivacyLevel () const
	{
		return PrivacyLevel_;
	}

	void AlbumSettingsDialog::validate ()
	{
		const bool isValid = !GetName ().isEmpty ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isValid);
	}

	void AlbumSettingsDialog::on_PhotosPrivacy__currentIndexChanged (int index)
	{
		switch (index)
		{
		case 0:
			PrivacyLevel_ = 255;
			break;
		case 1:
			PrivacyLevel_ = 254;
			break;
		case 2:
		{
			SelectGroupsDialog dlg (Login_, Account_);
			if (dlg.exec () == QDialog::Rejected)
				break;

			PrivacyLevel_ = dlg.GetSelectedGroupId ();
			break;
		}
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown index of photo privacy level";
			break;
		}
	}

}
}
}
