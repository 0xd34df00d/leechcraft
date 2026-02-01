/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransfersection.h"
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "chattab.h"
#include "transferjobmanager.h"
#include "xmlsettingsmanager.h"
#include "plugins/bittorrent/core.h"

namespace LC::Azoth
{
	FileTransferSection::FileTransferSection (QToolButton& eventsButton, ChatTab& tab, TransferJobManager& transfers)
	: QObject { &tab }
	, EventsButton_ { eventsButton }
	, Tab_ { tab }
	, Transfers_ { transfers }
	{
		const auto e = Tab_.GetEntry<ICLEntry> ();
		const auto tm = e->GetParentAccount ()->GetTransferManager ();
		if (qobject_cast<ITransferManager*> (tm))
		{
			connect (tm,
					SIGNAL (fileOffered (QObject*)),
					this,
					SLOT (handleFileOffered (QObject*)));

			for (const auto object : Transfers_.GetPendingIncomingJobsFor (Tab_.GetEntryID ()))
				handleFileOffered (object);
		}

		connect (&Transfers_,
				&TransferJobManager::jobNoLongerOffered,
				this,
				&FileTransferSection::HandleFileNoLongerOffered);
	}

	void FileTransferSection::handleFileOffered (QObject *jobObj)
	{
		const auto job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << jobObj << "is not an ITransferJob";
			return;
		}

		if (job->GetSourceID () != Tab_.GetEntryID ())
			return;

		EventsButton_.show ();

		const auto& text = tr ("File offered: %1.")
				.arg (job->GetName ());
		const auto act = EventsButton_.menu ()->addAction (text, this, [this, job] { HandleOfferActionTriggered (job); });
		act->setToolTip (job->GetComment ());
	}

	void FileTransferSection::HandleFileNoLongerOffered (QObject *jobObj)
	{
		for (const auto action : EventsButton_.menu ()->actions ())
			if (action->data ().value<QObject*> () == jobObj)
			{
				action->deleteLater ();
				break;
			}

		if (EventsButton_.menu ()->actions ().count () == 1)
			EventsButton_.hide ();
	}

	void FileTransferSection::HandleOfferActionTriggered (ITransferJob *job)
	{
		auto text = tr ("Would you like to accept or reject file transfer request for file %1?")
				.arg (job->GetName ());
		if (!job->GetComment ().isEmpty ())
		{
			text += "<br /><br />" + tr ("The file description is:") + "<br /><br /><em>";
			auto comment = job->GetComment ().toHtmlEscaped ();
			comment.replace ("\n", "<br />");
			text += comment + "</em>";
		}

		const auto questResult = QMessageBox::question (&Tab_,
				tr ("File transfer request"), text,
				QMessageBox::Save | QMessageBox::Abort | QMessageBox::Cancel);

		if (questResult == QMessageBox::Cancel)
			return;

		auto& jobObj = dynamic_cast<QObject&> (*job);
		if (questResult == QMessageBox::Abort)
			Transfers_.DenyJob (&jobObj);
		else
		{
			const QString& path = QFileDialog::getExistingDirectory (&Tab_,
					tr ("Select save path for incoming file"),
					XmlSettingsManager::Instance ().property ("DefaultXferSavePath").toString ());
			if (path.isEmpty ())
				return;

			Transfers_.AcceptJob (&jobObj, path);
		}
	}
}
