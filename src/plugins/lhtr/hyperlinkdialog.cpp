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

#include "hyperlinkdialog.h"
#include <QPushButton>
#include <QtDebug>

namespace LeechCraft
{
namespace LHTR
{
	HyperlinkDialog::HyperlinkDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.Link_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (checkCanAccept ()));
		connect (Ui_.Text_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (checkCanAccept ()));
		checkCanAccept ();
	}

	QString HyperlinkDialog::GetLink () const
	{
		return Ui_.Link_->text ();
	}

	QString HyperlinkDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}

	QString HyperlinkDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QString HyperlinkDialog::GetTarget () const
	{
		return Ui_.Target_->currentText ();
	}

	void HyperlinkDialog::checkCanAccept ()
	{
		const bool can = !GetLink ().isEmpty () && !GetText ().isEmpty ();
		qDebug () << Q_FUNC_INFO << can << GetLink () << GetText ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (can);
	}
}
}
