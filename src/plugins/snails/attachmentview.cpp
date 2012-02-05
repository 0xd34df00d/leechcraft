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

#include "attachmentview.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include "proto/Imap/Network/MsgPartNetAccessManager.h"
#include "proto/Imap/Network/FileDownloadManager.h"
#include "proto/Imap/Model/ItemRoles.h"
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	AttachmentView::AttachmentView (const QModelIndex& idx,
			std::shared_ptr<Imap::Network::MsgPartNetAccessManager> mpnam, QWidget *parent)
	: QWidget (parent)
	, MPNAM_ (mpnam)
	, FDM_ (new Imap::Network::FileDownloadManager (MPNAM_.get (), MPNAM_.get (), idx))
	{
		auto lay = new QHBoxLayout ();
		setLayout (lay);

		auto label = new QLabel (tr ("Attachment %1 (%2)")
					.arg (idx.data (Imap::Mailbox::RolePartFileName).toString ())
					.arg (idx.data (Imap::Mailbox::RolePartMimeType).toString ()));
		lay->addWidget (label);

		auto dlButton = new QPushButton (tr ("Download"));
		dlButton->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
		connect (dlButton,
				SIGNAL (released ()),
				FDM_,
				SLOT (slotDownloadNow ()));
		lay->addWidget (dlButton);

		connect (FDM_,
				SIGNAL (fileNameRequested (QString*)),
				this,
				SLOT (handleFilenameRequested (QString*)));
	}

	void AttachmentView::handleFilenameRequested (QString *filename)
	{
		*filename = QFileDialog::getSaveFileName (this,
				tr ("Select save path"),
				QDir::homePath ());
	}
}
}
