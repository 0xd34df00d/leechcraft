/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "urlframe.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			URLFrame::URLFrame (QWidget *parent)
			: QFrame (parent)
			{
				Ui_.setupUi (this);
			}

			QLineEdit* URLFrame::GetEdit () const
			{
				return Ui_.URLEdit_;
			}

			void URLFrame::SetFavicon (const QIcon& icon)
			{
				QPixmap pixmap = icon.pixmap (Ui_.FaviconLabel_->size ());
				Ui_.FaviconLabel_->setPixmap (pixmap);
			}

			void URLFrame::AddWidget (QWidget *widget)
			{
				layout ()->addWidget (widget);
			}

			void URLFrame::RemoveWidget (QWidget *widget)
			{
				layout ()->removeWidget (widget);
			}

			void URLFrame::on_URLEdit__returnPressed ()
			{
				if (Ui_.URLEdit_->IsCompleting () ||
						Ui_.URLEdit_->text ().isEmpty ())
					return;

				emit load (Ui_.URLEdit_->text ());
			}
		};
	};
};
