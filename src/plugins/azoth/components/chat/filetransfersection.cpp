/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransfersection.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "util/azoth/emitters/transfermanager.h"
#include "components/transfers/transferjobmanager.h"
#include "chattab.h"
#include "fileofferedpane.h"

namespace LC::Azoth
{
	FileTransferSection::FileTransferSection (ChatTab& tab, TransferJobManager& transfers)
	: QObject { &tab }
	, Tab_ { tab }
	, Transfers_ { transfers }
	{
		const auto e = Tab_.GetEntry<ICLEntry> ();
		if (const auto itm = qobject_cast<ITransferManager*> (e->GetParentAccount ()->GetTransferManager ()))
		{
			connect (&itm->GetTransferManagerEmitter (),
					&Emitters::TransferManager::fileOffered,
					this,
					&FileTransferSection::HandleFileOffered);

			for (const auto& offer : Transfers_.GetIncomingOffers (Tab_.GetEntryID ()))
				HandleFileOffered (offer);
		}
	}

	void FileTransferSection::HandleFileOffered (const IncomingOffer& offer)
	{
		if (offer.EntryId_ == Tab_.GetEntryID ())
			Tab_.AddActionPane (*new FileOfferedPane { offer, Transfers_, &Tab_ });
	}
}
