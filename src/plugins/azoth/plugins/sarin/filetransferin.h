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

class QFile;

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class FileTransferIn final : public FileTransferBase
	{
		const quint32 FriendNum_;
		const quint32 FileNum_;

		QString Filename_;
		const quint64 Filesize_;

		std::shared_ptr<QFile> File_;
	public:
		FileTransferIn (const QString& azothId,
				const QByteArray& pubkey,
				quint32 friendNum,
				quint32 fileNum,
				quint64 fileSize,
				const QString& offeredName,
				const std::shared_ptr<ToxThread>& thread,
				QObject *parent = nullptr);

		QString GetName () const override;
		qint64 GetSize () const override;
		TransferDirection GetDirection () const override;

		void Accept (const QString&) override;
		void Abort () override;

		void HandleData (quint32, quint32, const QByteArray&, uint64_t);
		void HandleFileControl (uint32_t, uint32_t, int);
	};
}
}
}
