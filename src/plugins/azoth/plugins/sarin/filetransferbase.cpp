/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransferbase.h"

namespace LC::Azoth::Sarin
{
	FileTransferBase::FileTransferBase (Pubkey pubkey, const std::shared_ptr<ToxRunner>& tox, QObject *parent)
	: QObject { parent }
	, PubKey_ { pubkey }
	, Tox_ { tox }
	{
	}

	Emitters::TransferJob& FileTransferBase::GetTransferJobEmitter ()
	{
		return Emitter_;
	}
}
