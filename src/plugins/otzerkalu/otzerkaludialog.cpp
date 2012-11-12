/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2011  Georg Rudoy
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

#include "otzerkaludialog.h"

namespace LeechCraft
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
