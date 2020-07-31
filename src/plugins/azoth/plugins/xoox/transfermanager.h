/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/azoth/itransfermanager.h>

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

		ClientConnection& Conn_;
		QXmppTransferManager& Manager_;
		GlooxAccount& Account_;
	public:
		TransferManager (QXmppTransferManager&, ClientConnection&, GlooxAccount&);

		bool IsAvailable () const override;
		QObject* SendFile (const QString&, const QString&, const QString&, const QString&) override;
		GlooxAccount* GetAccount () const;
	signals:
		void fileOffered (QObject*) override;
	};
}
}
}
