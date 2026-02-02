/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/azoth/itransfermanager.h>
#include <util/azoth/emitters/transfermanager.h>

class QFile;

class QXmppTransferJob;

namespace LC::Azoth::Xoox
{
	class TransferManager;

	class TransferJob : public QObject
					  , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferJob)

		Emitters::TransferJob Emitter_;

		std::unique_ptr<QFile> SaveFile_;
		std::unique_ptr<QXmppTransferJob> Job_;
	public:
		explicit TransferJob (std::unique_ptr<QXmppTransferJob>);
		explicit TransferJob (std::unique_ptr<QXmppTransferJob>, const QString& out);
		~TransferJob () override;

		Emitters::TransferJob& GetTransferJobEmitter () override;
		void Abort () override;
	};
}
