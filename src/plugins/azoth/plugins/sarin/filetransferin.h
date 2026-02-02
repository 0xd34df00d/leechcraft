/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "filetransferbase.h"
#include "types.h"

class QFile;

namespace LC::Azoth::Sarin
{
	class FileTransferIn final : public FileTransferBase
	{
		const quint32 FriendNum_;
		const quint32 FileNum_;
		const quint64 Filesize_;
		std::shared_ptr<QFile> File_;
	public:
		FileTransferIn (Pubkey pubkey,
				quint32 friendNum,
				quint32 fileNum,
				quint64 fileSize,
				const std::shared_ptr<ToxRunner>& tox,
				QObject *parent = nullptr);

		void Abort () override;

		void Accept (const QString&);

		void HandleData (quint32, quint32, const QByteArray&, uint64_t);
		void HandleFileControl (uint32_t, uint32_t, int);
	};
}
