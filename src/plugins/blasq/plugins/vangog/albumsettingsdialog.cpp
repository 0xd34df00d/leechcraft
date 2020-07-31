/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumsettingsdialog.h"
#include <QPushButton>
#include <util/gui/clearlineeditaddon.h>

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	AlbumSettingsDialog::AlbumSettingsDialog (const QString& name,
			ICoreProxy_ptr proxy, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.Name_->setText (name);

		new Util::ClearLineEditAddon (proxy, Ui_.Name_);
		new Util::ClearLineEditAddon (proxy, Ui_.Desc_);

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

	QString AlbumSettingsDialog::GetDesc () const
	{
		return Ui_.Desc_->text ();
	}

	int AlbumSettingsDialog::GetPrivacyLevel () const
	{
		return Ui_.PhotosPrivacy_->currentIndex ();
	}

	void AlbumSettingsDialog::validate ()
	{
		const bool isValid = !GetName ().isEmpty ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isValid);
	}
}
}
}
