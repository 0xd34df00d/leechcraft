/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagedialog.h"
#include <QFileDialog>
#include <QUrl>

namespace LC::LHTR
{
	ImageDialog::ImageDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		connect (Ui_.TypeEmbed_,
				&QRadioButton::toggled,
				Ui_.Browse_,
				&QWidget::setEnabled);
		connect (Ui_.Browse_,
				&QPushButton::released,
				[this]
				{
					const auto& path = QFileDialog::getOpenFileName (this,
							tr ("Select image"),
							QDir::homePath ());
					if (path.isEmpty ())
						return;

					Ui_.Path_->setText (QUrl::fromLocalFile (path).toString ());

					QImage image { path };
					if (image.isNull ())
					{
						Ui_.Width_->setValue (0);
						Ui_.Height_->setValue (0);
					}
					else
					{
						Ui_.Width_->setValue (image.width ());
						Ui_.Height_->setValue (image.height ());
					}
				});
	}

	QString ImageDialog::GetPath () const
	{
		return Ui_.Path_->text ();
	}

	QString ImageDialog::GetAlt () const
	{
		return Ui_.Alt_->text ();
	}

	int ImageDialog::GetWidth () const
	{
		return Ui_.Width_->value ();
	}

	int ImageDialog::GetHeight () const
	{
		return Ui_.Height_->value ();
	}

	QString ImageDialog::GetFloat () const
	{
		switch (Ui_.Float_->currentIndex ())
		{
		case 1:
			return QStringLiteral ("left");
		case 2:
			return QStringLiteral ("right");
		default:
			return QStringLiteral ("none");
		}
	}
}
