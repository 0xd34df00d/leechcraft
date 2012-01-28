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

#include "transferjob.h"
#include <QFileInfo>
#include <QtDebug>
#include <msn/notificationserver.h>
#include "msnaccount.h"
#include "msnbuddyentry.h"
#include "zheetutil.h"
#include "callbacks.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	TransferJob::TransferJob (const MSN::fileTransferInvite& invite, Callbacks *cb, MSNAccount *acc)
	: QObject (acc)
	, ID_ (invite.sessionId)
	, A_ (acc)
	, CB_ (cb)
	, Buddy_ (acc->GetBuddy (ZheetUtil::FromStd (invite.userPassport)))
	, Dir_ (TransferDirection::TDIn)
	, Filename_ (ZheetUtil::FromStd (invite.friendlyname))
	, Done_ (0)
	, Total_ (invite.filesize)
	, State_ (TransferState::TSOffer)
	{
	}

	TransferJob::TransferJob (uint id, const QString& filename,
			MSNBuddyEntry *entry, Callbacks *cb, MSNAccount *acc)
	: QObject (acc)
	, ID_ (id)
	, A_ (acc)
	, CB_ (cb)
	, Buddy_ (entry)
	, Dir_ (TransferDirection::TDOut)
	, Filename_ (filename)
	, Done_ (0)
	, Total_ (QFileInfo (filename).size ())
	, State_ (TransferState::TSOffer)
	{
		connect (CB_,
				SIGNAL (fileTransferProgress (uint, quint64, quint64)),
				this,
				SLOT (handleProgress (uint, quint64, quint64)));
		connect (CB_,
				SIGNAL (fileTransferFailed (uint)),
				this,
				SLOT (handleFailed (uint)));
		connect (CB_,
				SIGNAL (fileTransferFinished (uint)),
				this,
				SLOT (handleFinished (uint)));
		connect (CB_,
				SIGNAL (fileTransferGotResponse (uint, bool)),
				this,
				SLOT (handleGotResponse (uint, bool)));
	}

	QString TransferJob::GetSourceID () const
	{
		return Buddy_->GetEntryID ();
	}
	
	QString TransferJob::GetName () const
	{
		return Filename_;
	}

	qint64 TransferJob::GetSize () const
	{
		return Total_;
	}

	TransferDirection TransferJob::GetDirection () const
	{
		return Dir_;
	}
	
	void TransferJob::Accept (const QString& out)
	{
		if (Dir_ == TDOut)
		{
			qWarning () << Q_FUNC_INFO
					<< "can't accept outgoing transfer job";
			return;
		}
		
		auto sb = GetSB ();
		if (!sb)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null SB for"
					<< Buddy_->GetHumanReadableID ();
			return;
		}
		
		Filename_ = out;
		sb->fileTransferResponse (ID_, ZheetUtil::ToStd (out), true);
	}
	
	void TransferJob::Abort ()
	{
		auto sb = GetSB ();
		if (!sb)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null SB for"
					<< Buddy_->GetHumanReadableID ();
			return;
		}

		if (State_ != TSOffer || Dir_ == TDOut)
			sb->cancelFileTransfer (ID_);
		else if (Dir_ == TDIn)
			sb->fileTransferResponse (ID_, ZheetUtil::ToStd (Filename_), false);
	}
	
	MSN::SwitchboardServerConnection* TransferJob::GetSB () const
	{
		const auto& id = ZheetUtil::ToStd (Buddy_->GetHumanReadableID ());
		return A_->GetNSConnection ()->switchboardWithOnlyUser (id);
	}
	
	void TransferJob::handleProgress (uint id, quint64 done, quint64 total)
	{
		if (ID_ != id)
			return;
		
		Done_ = done;
		Total_ = total;
		emit transferProgress (done, total);
	}
	
	void TransferJob::handleFailed (uint id)
	{
		if (ID_ != id)
			return;
		
		State_ = TSFinished;
		emit errorAppeared (TEProtocolError, QString ());
	}

	void TransferJob::handleFinished (uint id)
	{
		if (ID_ != id)
			return;
		
		State_ = TSFinished;
		emit stateChanged (State_);
	}

	void TransferJob::handleGotResponse (uint id, bool resp)
	{
		if (ID_ != id)
			return;
		
		if (resp)
		{
			State_ = TSTransfer;
			emit stateChanged (State_);
		}
		else
		{
			State_ = TSFinished;
			emit errorAppeared (TEAborted, QString ());
		}
	}
}
}
}
