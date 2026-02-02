/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/azoth/itransfermanager.h>
#include <util/azoth/emitters/transfermanager.h>
#include "types.h"

namespace LC::Azoth::Sarin
{
	class ToxRunner;

	class FileTransferBase : public QObject
						   , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferJob)
	protected:
		Emitters::TransferJob Emitter_;
		const Pubkey PubKey_;
		const std::shared_ptr<ToxRunner> Tox_;
	public:
		FileTransferBase (Pubkey pubkey, const std::shared_ptr<ToxRunner>& tox, QObject *parent = nullptr);

		Emitters::TransferJob& GetTransferJobEmitter () override;
	};
}
