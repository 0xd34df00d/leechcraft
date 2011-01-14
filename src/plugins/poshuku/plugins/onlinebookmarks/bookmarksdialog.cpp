/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "bookmarksdialog.h"
#include <QStandardItemModel>
#include <QPushButton>
#include "core.h"
#include "syncbookmarks.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	BookmarksDialog::BookmarksDialog (QWidget* parent, Qt::WindowFlags f)
	: QDialog (parent, f)
	{
		Ui_.setupUi (this);
		
		if (XmlSettingsManager::Instance ()->Property ("ShowServices", false).toBool ())
			Ui_.ServicesView_->show ();
		else
			Ui_.ServicesView_->hide ();;
		
		connect (Ui_.Buttons_->button (QDialogButtonBox::Ok), 
				SIGNAL (clicked (bool)),
				this,
				SLOT (sendBookmark (bool)));
		
		connect (Ui_.Buttons_->button (QDialogButtonBox::No), 
				SIGNAL (clicked (bool)),
				this,
				SLOT (rejectSendBookmark (bool)));
		
		connect (Ui_.Buttons_->button (QDialogButtonBox::YesToAll), 
				SIGNAL (clicked (bool)),
				this,
				SLOT (sendBookmarkWithoutConfirm (bool)));
	}

	void BookmarksDialog::SetBookmark (const QString& title, const QString& url, const QStringList& tags)
	{
		Ui_.Title_->setText (title);
		Ui_.URL_->setText (url);
		Ui_.Tags_->setText (tags.join (","));
		Ui_.Ask_->setText (tr ("Please check the services you would like to add the bookmark %1 to, if any.").arg (url));
		Ui_.ServicesView_->setModel (Core::Instance ().GetServiceModel());
	}

	void BookmarksDialog::SendBookmark (bool checked)
	{
		Core::Instance ().GetBookmarksSyncManager ()->
				uploadBookmarks (Ui_.Title_->text (), Ui_.URL_->text (), Ui_.Tags_->text ().split (","));
	}

	void BookmarksDialog::rejectSendBookmark (bool checked)
	{
		QDialog::reject ();
	}

	void BookmarksDialog::sendBookmarkWithoutConfirm (bool checked)
	{

	}
}
}
}
}
}
