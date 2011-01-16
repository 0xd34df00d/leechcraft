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
#include "settings.h"
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
		
		Ui_.ServicesView_->setVisible (XmlSettingsManager::Instance ()->
				Property ("ShowServices", false).toBool ());
		
		connect (Ui_.Buttons_->button (QDialogButtonBox::YesToAll), 
				SIGNAL (clicked (bool)),
				this,
				SLOT (sendBookmarkWithoutConfirm (bool)));
	}

	void BookmarksDialog::SetBookmark (const QString& title, const QString& url, const QStringList& tags)
	{
		Ui_.Title_->setText (title);
		Ui_.URL_->setText (url);
		Ui_.Tags_->setText (tags.join (";"));
		Ui_.Ask_->setText (tr ("Please check the services you would like to add the bookmark %1 to, if any.")
				.arg (url));
		Ui_.ServicesView_->setModel (Core::Instance ().GetServiceModel());
	}

	void BookmarksDialog::SendBookmark ()
	{
		Core::Instance ().GetBookmarksSyncManager ()->
				uploadBookmarksAction (Ui_.Title_->text (), Ui_.URL_->text (), 
				Core::Instance ().SanitizeTagsList (Ui_.Tags_->text ()
				.split (';', QString::SkipEmptyParts)));
	}

	void BookmarksDialog::sendBookmarkWithoutConfirm (bool checked)
	{
		XmlSettingsManager::Instance ()->setProperty ("ConfirmSend", true);
	}
}
}
}
}
}
