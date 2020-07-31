/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2011  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "otzerkaludialog.h"

namespace LC
{
namespace Otzerkalu
{
	OtzerkaluDialog::OtzerkaluDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.SaveDirLineEdit_->setText (QDir::homePath ());
	}

	int OtzerkaluDialog::GetRecursionLevel () const
	{
		return Ui_.RecursionLevel_->value ();
	}

	QString OtzerkaluDialog::GetDir () const
	{
		return Ui_.SaveDirLineEdit_->text ();
	}

	bool OtzerkaluDialog::FetchFromExternalHosts () const
	{
		return Ui_.FromOtherSite_->isChecked ();
	}

	void OtzerkaluDialog::on_ChooseDirButton__clicked ()
	{
		QString saveDir = QFileDialog::getExistingDirectory (this,
				tr ("Save into a directory"),
				QDir::homePath (),
				QFileDialog::ShowDirsOnly |
						QFileDialog::DontResolveSymlinks);
		if (saveDir.isEmpty ())
			return;

		Ui_.SaveDirLineEdit_->setText (saveDir);
	}
}
}
