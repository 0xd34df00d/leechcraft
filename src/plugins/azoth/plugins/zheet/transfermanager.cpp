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

#include "transfermanager.h"
#include "msnaccount.h"
#include "sbmanager.h"
#include "transferjob.h"
#include "callbacks.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	TransferManager::TransferManager (Callbacks *cb, MSNAccount *parent)
	: QObject (parent)
	, A_ (parent)
	, CB_ (cb)
	, SessID_ (0)
	{
		connect (CB_,
				SIGNAL (fileTransferSuggested (MSN::fileTransferInvite)),
				this,
				SLOT (handleSuggestion (MSN::fileTransferInvite)));
	}
	
	QObject* TransferManager::SendFile (const QString& id,
			const QString&, const QString& name)
	{
		MSNBuddyEntry *buddy = A_->GetBuddy (id);
		A_->GetSBManager ()->SendFile (name, ++SessID_, buddy);
		return new TransferJob (SessID_, name, buddy, CB_, A_);
	}
	
	void TransferManager::handleSuggestion (MSN::fileTransferInvite fti)
	{
		TransferJob *job = new TransferJob (fti, CB_, A_);
		emit fileOffered (job);
	}
}
}
}
