/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_TRANSFERJOB_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_TRANSFERJOB_H
#include <interfaces/azoth/itransfermanager.h>
#include <QXmppTransferManager.h>

class QXmppTransferJob;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class TransferManager;

	class TransferJob : public QObject
					  , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferJob)

		QXmppTransferJob *Job_;
		TransferManager *Manager_;
	public:
		TransferJob (QXmppTransferJob*, TransferManager*);

		QString GetSourceID () const;
		QString GetName () const;
		qint64 GetSize () const;
		QString GetComment () const;
		TransferDirection GetDirection () const;
		void Accept (const QString& out);
		void Abort ();
	private slots:
		void handleErrorAppeared (QXmppTransferJob::Error);
		void handleStateChanged (QXmppTransferJob::State);
	signals:
		void transferProgress (qint64 done, qint64 total);
		void errorAppeared (TransferError error, const QString& msg);
		void stateChanged (TransferState state);
	};
}
}
}

#endif
