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

#include "addeditentrydialog.h"
#include <QColorDialog>
#include <QtDebug>
#include <QMessageBox>
#include "ljprofile.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	AddEditEntryDialog::AddEditEntryDialog (LJProfile *profile, QWidget *parent)
	: QDialog (parent)
	, Profile_ (profile)
	, BackgroundColor_ ("#ffffff")
	, ForegroundColor_ ("#000000")
	{
		Ui_.setupUi (this);

		Ui_.AddTypeEntry_->setItemData (0, ATEFriend, AddType);
		Ui_.AddTypeEntry_->setItemData (1, ATEGroup, AddType);

		Ui_.BackgroundColorLabel_->setMinimumWidth (QApplication::fontMetrics ()
				.width (" #RRGGBB "));
		Ui_.ForegroundColorLabel_->setMinimumWidth (QApplication::fontMetrics ()
				.width (" #RRGGBB "));

		Ui_.Groups_->addItem (tr ("Not defined"));
		Ui_.Groups_->setItemData (Ui_.Groups_->count (), -1, GroupRoles::RealGroupId);
		for (const auto& frGroup : Profile_->GetFriendGroups ())
		{
			Ui_.Groups_->addItem (frGroup.Name_);
			Ui_.Groups_->setItemData (Ui_.Groups_->count () - 1,
					frGroup.RealId_,
					GroupRoles::RealGroupId);
		}
	}

	QString AddEditEntryDialog::GetUserName () const
	{
		return Ui_.Username_->text ();
	}

	void AddEditEntryDialog::SetUserName (const QString& name)
	{
		Ui_.Username_->setText (name);
	}

	QString AddEditEntryDialog::GetBackgroundColorName () const
	{
		return BackgroundColor_.name ();
	}

	namespace
	{
		void DrawColorPixmap (QLabel *label, const QColor& color)
		{
			int height = QApplication::fontMetrics ().height ();
			int width = 1.62 * height;
			QPixmap pixmap (width, height);
			pixmap.fill (color);
			label->setPixmap (pixmap);
		}
	}

	void AddEditEntryDialog::SetBackgroundColor (const QColor& clr)
	{
		BackgroundColor_ = clr;
		DrawColorPixmap (Ui_.BackgroundColorLabel_, BackgroundColor_);
	}

	QString AddEditEntryDialog::GetForegroundColorName () const
	{
		return ForegroundColor_.name ();
	}

	void AddEditEntryDialog::SetForegroundColor (const QColor& clr)
	{
		ForegroundColor_ = clr;
		DrawColorPixmap (Ui_.ForegroundColorLabel_, ForegroundColor_);
	}

	uint AddEditEntryDialog::GetGroupRealId () const
	{
		return Ui_.Groups_->itemData (Ui_.Groups_->currentIndex (),
				GroupRoles::RealGroupId).toUInt ();
	}

	void AddEditEntryDialog::SetGroup (uint id)
	{
		for (int i = 0, count = Ui_.Groups_->count (); i < count; ++i)
		{
			if (Ui_.Groups_->itemData (i, GroupRoles::RealGroupId).toUInt () == id)
			{
				Ui_.Groups_->setCurrentIndex (i);
				return;
			}
		}

		Ui_.Groups_->setCurrentIndex (0);
	}

	QString AddEditEntryDialog::GetGroupName () const
	{
		return Ui_.GroupName_->text ();
	}
	
	void AddEditEntryDialog::SetGroupName (const QString& name)
	{
		Ui_.GroupName_->setText (name);
	}
	
	bool AddEditEntryDialog::GetAcccess () const
	{
		return Ui_.Public_->isChecked ();
	}
	
	void AddEditEntryDialog::SetAccess (bool isPublic)
	{
		Ui_.Public_->setChecked (isPublic);
	}

	AddTypeEntry AddEditEntryDialog::GetAddTypeEntry () const
	{
		return static_cast<AddTypeEntry> (Ui_.AddTypeEntry_->
				itemData (Ui_.AddTypeEntry_->currentIndex (), AddType).toInt ());
	}

	void AddEditEntryDialog::ShowAddTypePossibility (bool show)
	{
		Ui_.AddNewLabel_->setVisible (show);
		Ui_.AddTypeEntry_->setVisible (show);
	}

	void AddEditEntryDialog::SetCurrentAddTypeEntry (AddTypeEntry entry)
	{
		for (int i = 0, count = Ui_.AddTypeEntry_->count (); i < count; ++i)
		{
			if (Ui_.AddTypeEntry_->itemData (i, AddType).toInt () == entry)
			{
				Ui_.AddTypeEntry_->setCurrentIndex (i);
				return;
			}
		}

		Ui_.AddTypeEntry_->setCurrentIndex (0);
	}

	void AddEditEntryDialog::accept ()
	{
		if (!Ui_.AddTypeEntry_->currentIndex () &&
				Ui_.Username_->text ().isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("Add new friend."),
					tr ("Username must be defined."));
			Ui_.Username_->setFocus ();
			return;
		}
		else if (Ui_.AddTypeEntry_->currentIndex () == 1 &&
				Ui_.GroupName_->text ().isEmpty ())
		{
			QMessageBox::warning (this,
					tr ("Add new group."),
					tr ("Group name must be defined."));
			Ui_.GroupName_->setFocus ();
			return;
		}

		QDialog::accept ();
	}

	namespace
	{
		void SelectColor (const QString& text, const QString& initColor,
				QLabel *pixmapLabel, QColor *color, QWidget *parent)
		{
			const QColor& clr = QColorDialog::getColor (QColor (initColor),
					parent,
					text);
			if (!clr.isValid ())
				return;

			DrawColorPixmap (pixmapLabel, clr);
			*color = clr;
		}
	}

	void AddEditEntryDialog::on_SelectBackgroundColor__released ()
	{
		SelectColor (tr ("Select background color for new user."),
				"#ffffff",
				Ui_.BackgroundColorLabel_,
				&BackgroundColor_,
				this);
	}

	void AddEditEntryDialog::on_SelectForegroundColor__released ()
	{
		SelectColor (tr ("Select foreground color for new user."),
				"#000000",
				Ui_.ForegroundColorLabel_,
				&ForegroundColor_,
				this);
	}
}
}
}


