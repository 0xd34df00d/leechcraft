/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "pastedialog.h"
#include "pasteservicefactory.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	PasteDialog::PasteDialog (QWidget *parent)
	: QDialog (parent)
	, Choice_ (Choice::Cancel)
	{
		Ui_.setupUi (this);

		Q_FOREACH (const auto& info, PasteServiceFactory ().GetInfos ())
			Ui_.ServiceCombo_->addItem (info.Icon_, info.Name_);
	}

	PasteDialog::Choice PasteDialog::GetChoice () const
	{
		return Choice_;
	}

	PasteServiceFactory::Creator_f PasteDialog::GetCreator () const
	{
		return PasteServiceFactory ().GetInfos ().at (Ui_.ServiceCombo_->currentIndex ()).Creator_;
	}

	Highlight PasteDialog::GetHighlight () const
	{
		return static_cast<Highlight> (Ui_.HighlightCombo_->currentIndex ());
	}

	void PasteDialog::on_ButtonBox__clicked (QAbstractButton *button)
	{
		switch (Ui_.ButtonBox_->standardButton (button))
		{
		case QDialogButtonBox::Yes:
			Choice_ = Choice::Yes;
			break;
		case QDialogButtonBox::No:
			Choice_ = Choice::No;
			break;
		default:
			Choice_ = Choice::Cancel;
			break;
		}
	}
}
}
}
