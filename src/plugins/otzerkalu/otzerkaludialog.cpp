/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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
	OtzerkaluDialog::OtzerkaluDialog (QWidget* parent, Qt::WindowFlags f)
		: QDialog (parent, f)
		, ui (new Ui::OtzerkaluDialog)
	{
		ui->setupUi (this);
		
		ok = false;
		
		connect (ui->ChooseDirButton,
				SIGNAL (clicked ()),
				this,
				SLOT (getData ()));		
		connect (ui->CancelButton,
				SIGNAL (clicked ()),
				this,
				SLOT (close ()));
		connect (ui->OkButton,
				SIGNAL (clicked ()),
				this,
				SLOT (save ()));
	}

	OtzerkaluDialog::~OtzerkaluDialog ()
	{
		delete ui;
	}
	
	void OtzerkaluDialog::getData ()
	{
		saveDir = QFileDialog::getExistingDirectory (this, tr ("Save into a directory"),
				"~/",
				QFileDialog::ShowDirsOnly
				| QFileDialog::DontResolveSymlinks);
		if (saveDir.isEmpty ())
			return;
		ui->SaveDirLineEdit->setText (saveDir);
	}
	
	void OtzerkaluDialog::save()
	{
		recLevel = ui->RecursionLevel->value ();
		ok = true;
		close ();
	}
	
	int OtzerkaluDialog::getRecursionLevel ()
	{
		return recLevel;
	}
	
	QString OtzerkaluDialog::getDir ()
	{
		return saveDir;
	}
		
	bool OtzerkaluDialog::isOk ()
	{
		return ok;
	}
}
}