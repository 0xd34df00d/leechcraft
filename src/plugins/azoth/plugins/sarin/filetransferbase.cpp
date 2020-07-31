/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransferbase.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	FileTransferBase::FileTransferBase (const QString& azothId,
			const QByteArray& pubkey,
			const std::shared_ptr<ToxThread>& thread,
			QObject *parent)
	: QObject { parent }
	, AzothId_ { azothId }
	, PubKey_ { pubkey }
	, Thread_ { thread }
	{
	}

	QString FileTransferBase::GetSourceID () const
	{
		return AzothId_;
	}

	QString FileTransferBase::GetComment () const
	{
		return {};
	}
}
}
}
