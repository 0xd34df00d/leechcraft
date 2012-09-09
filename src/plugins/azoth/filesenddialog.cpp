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

#include "filesenddialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iwebfilestorage.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/itransfermanager.h"
#include "core.h"
#include "chattabsmanager.h"
#include "transferjobmanager.h"
#include "pendinguploadpaster.h"

namespace LeechCraft
{
namespace Azoth
{
	FileSendDialog::FileSendDialog (ICLEntry *entry, const QString& suggested, QWidget *parent)
	: QDialog (parent)
	, Entry_ (entry)
	, EntryVariant_ (Core::Instance ().GetChatTabsManager ()->GetActiveVariant (entry))
	, AccSupportsFT_ (false)
	{
		Ui_.setupUi (this);
		setAttribute (Qt::WA_DeleteOnClose);
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (send ()));
		show ();

		auto name = entry->GetEntryName ();
		if (name != entry->GetHumanReadableID ())
			name += " (" + entry->GetHumanReadableID () + ")";
		Ui_.TargetLabel_->setText (name);

		auto acc = qobject_cast<IAccount*> (Entry_->GetParentAccount ());
		if (acc->GetTransferManager ())
		{
			AccSupportsFT_ = true;
			Ui_.TransferMethod_->addItem (tr ("Protocol file transfer"));
		}

		FillSharers ();

		if (suggested.isEmpty ())
			on_FileBrowse__released ();
		else
			Ui_.FileEdit_->setText (suggested);
	}

	void FileSendDialog::FillSharers ()
	{
		auto sharers = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<IWebFileStorage*> ();
		QMap<QString, QObject*> variants;
		Q_FOREACH (auto sharerObj, sharers)
		{
			auto sharer = qobject_cast<IWebFileStorage*> (sharerObj);
			Q_FOREACH (const auto& var, sharer->GetServiceVariants ())
			{
				const int idx = Ui_.TransferMethod_->count ();
				Ui_.TransferMethod_->addItem (var);
				Pos2Sharer_ [idx] = { sharerObj, var };
			}
		}
	}

	void FileSendDialog::SendSharer (const SharerInfo& info)
	{
		const auto& filename = Ui_.FileEdit_->text ();

		auto sharer = qobject_cast<IWebFileStorage*> (info.Sharer_);
		sharer->UploadFile (filename, info.Service_);
		new PendingUploadPaster (info.Sharer_, Entry_, EntryVariant_, filename);
	}

	void FileSendDialog::SendProto ()
	{
		auto acc = qobject_cast<IAccount*> (Entry_->GetParentAccount ());
		auto xferMgr = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (!xferMgr)
		{
			qWarning () << Q_FUNC_INFO
					<< "null Xfer manager for"
					<< Entry_->GetObject ();
			return;
		}

		const auto& filename = Ui_.FileEdit_->text ();
		if (filename.isEmpty ())
			return;

		QObject *job = xferMgr->SendFile (Entry_->GetEntryID (), EntryVariant_, filename);
		if (!job)
		{
			Core::Instance ().SendEntity (Util::MakeNotification ("Azoth",
						tr ("Unable to send file to %1.")
							.arg (Entry_->GetEntryName ()),
						PCritical_));
			return;
		}
		Core::Instance ().GetTransferJobManager ()->HandleJob (job);
	}

	void FileSendDialog::send ()
	{
		const int idx = Ui_.TransferMethod_->currentIndex ();
		if (Pos2Sharer_.contains (idx))
			SendSharer (Pos2Sharer_ [idx]);
		else if (AccSupportsFT_)
			SendProto ();
		else
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("No file transfer methods available."));
	}

	void FileSendDialog::on_FileBrowse__released ()
	{
		const QString& filename = QFileDialog::getOpenFileName (0,
				tr ("Select file to send"));
		if (filename.isEmpty ())
			return;

		Ui_.FileEdit_->setText (filename);
	}
}
}
