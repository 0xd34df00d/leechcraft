/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <unordered_map>
#include <interfaces/azoth/itransfermanager.h>
#include <util/azoth/emitters/transfermanager.h>

class QXmppTransferManager;
class QXmppTransferJob;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class ClientConnection;

	class TransferManager : public QObject
						  , public ITransferManager
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferManager)

		Emitters::TransferManager Emitter_;

		ClientConnection& Conn_;
		QXmppTransferManager& Manager_;
		GlooxAccount& Account_;

		std::unordered_map<uint64_t, std::unique_ptr<QXmppTransferJob>> PendingJobs_;
		uint64_t JobIdGen_ = 0;
	public:
		TransferManager (QXmppTransferManager&, ClientConnection&, GlooxAccount&);

		Emitters::TransferManager& GetTransferManagerEmitter () override;
		bool IsAvailable () const override;
		ITransferJob* Accept (const IncomingOffer& offer, const QString& savePath) override;
		void Decline (const IncomingOffer&) override;
		ITransferJob* SendFile (const QString&, const QString&, const QString&, const QString&) override;

		GlooxAccount* GetAccount () const;
	private:
		void HandleQxmppJob (QXmppTransferJob*);
	};
}
}
}
