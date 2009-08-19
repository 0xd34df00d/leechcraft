/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "exportdialog.h"
#include <QFileDialog>
#include <QDir>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			ExportDialog::ExportDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			}
			
			ExportDialog::~ExportDialog ()
			{
			}
			
			QString ExportDialog::GetLocation () const
			{
				return Ui_.SaveLine_->text ();
			}
			
			bool ExportDialog::GetSettings () const
			{
				return Ui_.SettingsBox_->checkState () == Qt::Checked;
			}
			
			bool ExportDialog::GetActive () const
			{
				return Ui_.TorrentsBox_->checkState () == Qt::Checked;
			}
			
			void ExportDialog::on_BrowseButton__released ()
			{
				QString filename = QFileDialog::getSaveFileName (this,
						tr ("Save file"),
						QDir::homePath () + "/export.lcte",
						tr ("BitTorrent Exchange (*.lcte)"));
				if (filename.isEmpty ())
					return;
			
				Ui_.SaveLine_->setText (filename);
			}
			
		};
	};
};

