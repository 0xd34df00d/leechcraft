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

#include "tabwidget.h"
#include "core.h"
#include "tabmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			TabWidget::TabWidget (const QUrl& url,
					const QString& str,
					QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);

				Setup (Ui_.Left_);
				Setup (Ui_.Right_);

				Ui_.Right_->SetURL (url);
				Ui_.Left_->Navigate (str);
			}

			TabWidget::~TabWidget ()
			{
			}

			void TabWidget::Remove ()
			{
				Core::Instance ().GetTabManager ()->Remove (this);
			}

			QToolBar* TabWidget::GetToolBar () const
			{
				return 0;
			}

			void TabWidget::NewTabRequested ()
			{
			}

			QList<QAction*> TabWidget::GetTabBarContextMenuActions () const
			{
				return QList<QAction*> ();
			}

			void TabWidget::Setup (Pane *p)
			{
				connect (p,
						SIGNAL (downloadRequested (const QUrl&)),
						this,
						SLOT (handleDownloadRequested (const QUrl&)));
				connect (p,
						SIGNAL (uploadRequested (const QString&)),
						this,
						SLOT (handleUploadRequested (const QString&)));
			}

			Pane* TabWidget::Other (Pane *p)
			{
				if (p == Ui_.Left_)
					return Ui_.Right_;
				else if (p == Ui_.Right_)
					return Ui_.Left_;
				else
					return 0;
			}

			void TabWidget::handleDownloadRequested (const QUrl& url)
			{
				Pane *other = Other (static_cast<Pane*> (sender ()));
				// TODO we don't support FXP yet
				if (!other->IsLocal ())
					return;
				Core::Instance ().Add (url, other->GetString (), true);
			}

			void TabWidget::handleUploadRequested (const QString& str)
			{
				Pane *other = Other (static_cast<Pane*> (sender ()));
				if (other->IsLocal ())
					return;
				Core::Instance ().Add (str, QUrl (other->GetString ()));
			}
		};
	};
};

