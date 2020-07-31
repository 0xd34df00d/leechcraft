/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <interfaces/azoth/itransfermanager.h>

class QNetworkReply;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;
	class VkConnection;
	class VkEntry;

	class PendingUpload : public QObject
						, public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferJob)

		VkAccount * const Acc_;
		VkConnection * const Conn_;

		const QString Path_;
		const QString Comment_;

		const QPointer<VkEntry> Entry_;
	public:
		PendingUpload (VkEntry*, const QString& path, const QString& comment, VkAccount *acc);

		QString GetSourceID () const;
		QString GetName () const;
		qint64 GetSize () const;
		QString GetComment () const;
		TransferDirection GetDirection () const;

		void Accept (const QString&);
		void Abort ();
	private:
		void HandleGotServer (QNetworkReply*);
		void HandleSaved (QNetworkReply*);
	private slots:
		void handleUploadFinished ();
	signals:
		void transferProgress (qint64, qint64);
		void errorAppeared (TransferError, const QString&);
		void stateChanged (TransferState);
	};
}
}
}
