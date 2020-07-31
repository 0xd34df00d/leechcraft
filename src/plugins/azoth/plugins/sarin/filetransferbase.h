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

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class ToxThread;

	class FileTransferBase : public QObject
						   , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferJob)
	protected:
		const QString AzothId_;
		const QByteArray PubKey_;
		const std::shared_ptr<ToxThread> Thread_;
	public:
		FileTransferBase (const QString& azothId,
				const QByteArray& pubkey,
				const std::shared_ptr<ToxThread>& thread,
				QObject *parent = nullptr);

		QString GetSourceID () const override;
		QString GetComment () const override;
	signals:
		void transferProgress (qint64 done, qint64 total) override;
		void errorAppeared (TransferError error, const QString& msg) override;
		void stateChanged (TransferState state) override;
	};
}
}
}
