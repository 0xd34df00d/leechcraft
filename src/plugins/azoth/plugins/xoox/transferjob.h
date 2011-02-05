/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_TRANSFERJOB_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_TRANSFERJOB_H
#include <interfaces/itransfermanager.h>
#include <QXmppTransferManager.h>

class QXmppTransferJob;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class TransferJob : public QObject
					  , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ITransferJob);

		QXmppTransferJob *Job_;
	public:
		TransferJob (QXmppTransferJob*);

		QString GetSourceID () const;
		QString GetName () const;
		qint64 GetSize () const;
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
