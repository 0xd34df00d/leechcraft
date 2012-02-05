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

#include "inlinepartwidget.h"
#include <QModelIndex>
#include <QFileDialog>
#include <QtDebug>
#include "proto/Imap/Network/MsgPartNetAccessManager.h"
#include "proto/Imap/Network/FileDownloadManager.h"
#include "proto/Imap/Model/ItemRoles.h"

namespace LeechCraft
{
namespace Snails
{
	InlinePartWidget::InlinePartWidget (const QModelIndex& idx,
			std::shared_ptr<Imap::Network::MsgPartNetAccessManager> mpnam, QWidget *parent)
	: QWebView (parent)
	, MPNAM_ (mpnam)
	, FDM_ (new Imap::Network::FileDownloadManager (MPNAM_.get (), mpnam.get (), idx))
	{
		page ()->setNetworkAccessManager (mpnam.get ());

		settings ()->setAttribute (QWebSettings::JavascriptEnabled, false);
		settings ()->setAttribute (QWebSettings::JavaEnabled, false);
		settings ()->setAttribute (QWebSettings::PluginsEnabled, false);
		settings ()->setAttribute (QWebSettings::PrivateBrowsingEnabled, true);
		settings ()->setAttribute (QWebSettings::JavaEnabled, false);
		settings ()->setAttribute (QWebSettings::OfflineStorageDatabaseEnabled, false);
		settings ()->setAttribute (QWebSettings::OfflineWebApplicationCacheEnabled, false);
		settings ()->setAttribute (QWebSettings::LocalStorageDatabaseEnabled, false);

		page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);

		QUrl url;
		url.setScheme ("trojita-imap");
		url.setHost ("msg");
		url.setPath (idx.data (Imap::Mailbox::RolePartPathToPart).toString ());
		load (url);

		connect (FDM_,
				SIGNAL (fileNameRequested (QString*)),
				this,
				SLOT (handleFilenameRequested (QString*)));
	}

	void InlinePartWidget::handleFilenameRequested (QString *filename)
	{
		*filename = QFileDialog::getSaveFileName (this,
				tr ("Select save file name"),
				QDir::homePath ());
	}
}
}
