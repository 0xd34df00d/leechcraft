/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addeditentrydialog.h"
#include <QColorDialog>
#include <QtDebug>
#include <QMessageBox>
#include <util/sll/curry.h>
#include "ljprofile.h"
#include "selectgroupsdialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	AddEditEntryDialog::AddEditEntryDialog (LJProfile *profile, AddTypeEntry type, QWidget *parent)
	: QDialog (parent)
	, Profile_ (profile)
	, BackgroundColor_ ("#ffffff")
	, ForegroundColor_ ("#000000")
	, GroupMask_ (0)
	{
		Ui_.setupUi (this);

		Ui_.AddTypeEntry_->setItemData (0, ATEFriend, AddType);
		Ui_.AddTypeEntry_->setItemData (1, ATEGroup, AddType);

		const auto& fm = fontMetrics ();
		Ui_.BackgroundColorLabel_->setMinimumWidth (fm.horizontalAdvance (" #RRGGBB "));
		Ui_.ForegroundColorLabel_->setMinimumWidth (fm.horizontalAdvance (" #RRGGBB "));

		switch (type)
		{
		case ATEFriend:
			Ui_.AddTypeEntry_->setCurrentIndex (0);
			break;
		case ATEGroup:
			Ui_.AddTypeEntry_->setCurrentIndex (1);
			break;
		case ATENone:
		default:
			break;
		};
		Ui_.AddNewLabel_->setVisible (type == ATENone);
		Ui_.AddTypeEntry_->setVisible (type == ATENone);
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

	uint AddEditEntryDialog::GetGroupMask () const
	{
		return GroupMask_;
	}

	void AddEditEntryDialog::SetGroupMask (uint mask)
	{
		GroupMask_ = mask;
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
					"Blogique Metida",
					tr ("Username must be defined."));
			Ui_.Username_->setFocus ();
			return;
		}
		else if (Ui_.AddTypeEntry_->currentIndex () == 1 &&
				Ui_.GroupName_->text ().isEmpty ())
		{
			QMessageBox::warning (this,
					"Blogique Metida",
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
		SelectColor (tr ("Select background color for new user:"),
				"#ffffff",
				Ui_.BackgroundColorLabel_,
				&BackgroundColor_,
				this);
	}

	void AddEditEntryDialog::on_SelectForegroundColor__released ()
	{
		SelectColor (tr ("Select foreground color for new user:"),
				"#000000",
				Ui_.ForegroundColorLabel_,
				&ForegroundColor_,
				this);
	}

	void AddEditEntryDialog::on_SelectGroups__released ()
	{
		SelectGroupsDialog dlg (Profile_, GroupMask_);
		dlg.SetHeaderLabel (tr ("Add friend to groups:"));
		if (dlg.exec () == QDialog::Rejected)
			return;

		GroupMask_ = 0;
		for (uint id : dlg.GetSelectedGroupsIds ())
			GroupMask_ |= (1 << id);
	}

}
}
}


