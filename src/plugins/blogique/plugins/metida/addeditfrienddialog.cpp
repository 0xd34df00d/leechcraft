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

#include "addeditfrienddialog.h"
#include "ljprofile.h"
#include <QColorDialog>
#include <QMessageBox>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	AddEditFriendDialog::AddEditFriendDialog (LJProfile *profile, QWidget *parent)
	: QDialog (parent)
	, Profile_ (profile)
	{
		Ui_.setupUi (this);

		Ui_.BackgroundColorLabel_->setMinimumWidth (QApplication::fontMetrics ()
				.width (" #RRGGBB "));
		Ui_.ForegroundColorLabel_->setMinimumWidth (QApplication::fontMetrics ()
				.width (" #RRGGBB "));

		Ui_.Groups_->addItem (tr ("Not defined"));
		Ui_.Groups_->setItemData (Ui_.Groups_->count (), -1, GroupRoles::RealGroupId);
		for (const auto& frGroup : Profile_->GetFriendGroups ())
		{
			Ui_.Groups_->addItem (frGroup.Name_);
			Ui_.Groups_->setItemData (Ui_.Groups_->count (),
					frGroup.RealId_,
					GroupRoles::RealGroupId);
		}
	}

	QString AddEditFriendDialog::GetUserName () const
	{
		return Ui_.Username_->text ();
	}

	QString AddEditFriendDialog::GetBackgroundColorName () const
	{
		return BackgroundColor_.name ();
	}

	QString AddEditFriendDialog::GetForegroundColorName () const
	{
		return ForegroundColor_.name ();
	}

	uint AddEditFriendDialog::GetGroupRealId () const
	{
		return Ui_.Groups_->itemData (Ui_.Groups_->currentIndex (),
				GroupRoles::RealGroupId).toUInt ();
	}

	void AddEditFriendDialog::on_SelectBackgroundColor__released ()
	{
		const QColor& color = QColorDialog::getColor (QColor  ("#ffffff"),
				this,
				tr ("Select background color for new user."));
		if (!color.isValid ())
			return;

		int height = QApplication::fontMetrics ().height ();
		int width = 1.62 * height;
		QPixmap pixmap (width, height);
		pixmap.fill (color);
		Ui_.BackgroundColorLabel_->setPixmap (pixmap);
		BackgroundColor_ = color;
	}

	void AddEditFriendDialog::on_SelectForegroundColor__released ()
	{
		const QColor& color = QColorDialog::getColor (QColor  ("#ffffff"),
				this,
				tr ("Select foreground color for new user."));
		if (!color.isValid ())
			return;

		int height = QApplication::fontMetrics ().height ();
		int width = 1.62 * height;
		QPixmap pixmap (width, height);
		pixmap.fill (color);
		Ui_.ForegroundColorLabel_->setPixmap (pixmap);
		ForegroundColor_ = color;
	}

	void AddEditFriendDialog::accept ()
	{
		if (Ui_.Username_->text ().isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("Add new friend."),
					tr ("Username must be defined."));
			Ui_.Username_->setFocus ();
			return;
		}

		QDialog::accept ();
	}

}
}
}


