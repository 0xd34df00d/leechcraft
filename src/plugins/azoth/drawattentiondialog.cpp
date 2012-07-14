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

#include "drawattentiondialog.h"

namespace LeechCraft
{
namespace Azoth
{
	DrawAttentionDialog::DrawAttentionDialog (const QStringList& resources, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		
		if (resources.size () > 1)
		{
			Ui_.ResourceBox_->addItem (tr ("<all>"));
			Ui_.ResourceBox_->addItems (resources);
		}
		else
			Ui_.ResourceBox_->setEnabled (false);
	}
	
	QString DrawAttentionDialog::GetResource () const
	{
		if (!Ui_.ResourceBox_->count () ||
				!Ui_.ResourceBox_->currentIndex ())
			return QString ();
		
		return Ui_.ResourceBox_->currentText ();
	}
	
	QString DrawAttentionDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}
}
}
