/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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
	namespace Plugins
	{
		namespace Auscrie
		{
			ShooterDialog::ShooterDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			}

			ShooterDialog::Action ShooterDialog::GetAction () const
			{
				switch (Ui_.ActionBox_->currentIndex ())
				{
					case 0:
						return AUpload;
					case 1:
						return ASave;
					default:
						qWarning () << Q_FUNC_INFO
							<< Ui_.ActionBox_->currentIndex ()
							<< "unhandled";
						return ASave;
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
				return Ui_.QualityBox_->value ();
			}
		};
	};
};

