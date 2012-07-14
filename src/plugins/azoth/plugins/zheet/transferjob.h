/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_TRANSFERJOB_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_TRANSFERJOB_H
#include <QObject>
#include <msn/util.h>
#include <interfaces/azoth/itransfermanager.h>

namespace MSN
{
	class SwitchboardServerConnection;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNAccount;
	class MSNBuddyEntry;
	class Callbacks;

	class TransferJob : public QObject
					  , public ITransferJob
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ITransferJob);
		
		uint ID_;
		MSNAccount *A_;
		Callbacks *CB_;
		MSNBuddyEntry *Buddy_;
		
		TransferDirection Dir_;
		QString Filename_;
		quint64 Done_;
		quint64 Total_;
		
		TransferState State_;
	public:
		TransferJob (const MSN::fileTransferInvite&,
				Callbacks*, MSNAccount*);
		TransferJob (uint, const QString&,
				MSNBuddyEntry*, Callbacks*, MSNAccount*);

		QString GetSourceID () const;
		QString GetName () const;
		qint64 GetSize () const;
		TransferDirection GetDirection () const;
		void Accept (const QString&);
		void Abort ();
	private:
		MSN::SwitchboardServerConnection* GetSB () const;
	private slots:
		void handleProgress (uint, quint64, quint64);
		void handleFailed (uint);
		void handleFinished (uint);
		void handleGotResponse (uint, bool);
	signals:
		void transferProgress (qint64 done, qint64 total);
		void errorAppeared (TransferError error, const QString& msg);
		void stateChanged (TransferState state);
	};
}
}
}

#endif
