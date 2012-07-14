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

#include "shooterdialog.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Auscrie
{
	ShooterDialog::ShooterDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		on_Format__currentIndexChanged (Ui_.Format_->currentText ());
	}

	ShooterDialog::Action ShooterDialog::GetAction () const
	{
		switch (Ui_.ActionBox_->currentIndex ())
		{
		case 0:
		case 1:
		case 2:
			return Action::Upload;
		case 3:
			return Action::Save;
		default:
			qWarning () << Q_FUNC_INFO
					<< Ui_.ActionBox_->currentIndex ()
					<< "unhandled";
			return Action::Save;
		}
	}

	ShooterDialog::Mode ShooterDialog::GetMode () const
	{
		switch (Ui_.ModeBox_->currentIndex ())
		{
		case 0:
			return Mode::LCWindowOverlay;
		case 1:
			return Mode::LCWindow;
		case 2:
			return Mode::CurrentScreen;
		case 3:
			return Mode::WholeDesktop;
		default:
			qWarning () << Q_FUNC_INFO
					<< Ui_.ModeBox_->currentIndex ()
					<< "unhandled";
			return Mode::LCWindowOverlay;
		}
	}

	bool ShooterDialog::ShouldHide () const
	{
		return Ui_.HideThis_->checkState () == Qt::Checked;
	}

	Poster::HostingService ShooterDialog::GetHostingService () const
	{
		switch (Ui_.ActionBox_->currentIndex ())
		{
		case 0:
			return Poster::DumpBitcheeseNet;
		case 1:
			return Poster::SavepicRu;
		case 2:
			return Poster::ImagebinCa;
		default:
			qWarning () << Q_FUNC_INFO
					<< Ui_.ActionBox_->currentIndex ()
					<< "unhandled, defaulting to imagebin.ca";
			return Poster::ImagebinCa;
		}
	}

	int ShooterDialog::GetTimeout () const
	{
		return Ui_.Timeout_->value ();
	}

	QString ShooterDialog::GetFormat () const
	{
		return Ui_.Format_->currentText ();
	}

	int ShooterDialog::GetQuality () const
	{
		const int val = Ui_.QualityBox_->value ();
		return Ui_.Format_->currentText () == "JPG" ?
				val :
				100 - val;
	}

	void ShooterDialog::on_Format__currentIndexChanged (const QString& str)
	{
		if (str == "JPG")
			Ui_.SettingLabel_->setText ("Quality:");
		else
			Ui_.SettingLabel_->setText ("Compression:");
	}
}
}
