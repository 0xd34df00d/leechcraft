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

#include <QFileDialog>
#include "movetorrentfiles.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			MoveTorrentFiles::MoveTorrentFiles (const QString& old, QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				Ui_.OldLocation_->setText (old);
				Ui_.NewLocation_->setText (old);
			}
			
			QString MoveTorrentFiles::GetNewLocation () const
			{
				return Ui_.NewLocation_->text ();
			}
			
			void MoveTorrentFiles::on_Browse__released ()
			{
				QString dir = QFileDialog::getExistingDirectory (this, tr ("New location"),
						Ui_.NewLocation_->text ());
				if (dir.isEmpty () || dir == Ui_.NewLocation_->text ())
					return;
				Ui_.NewLocation_->setText (dir);
			}
			
		};
	};
};

