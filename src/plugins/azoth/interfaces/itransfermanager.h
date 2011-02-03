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

#ifndef PLUGINS_AZOTH_INTERFACES_ITRANSFERMANAGER_H
#define PLUGINS_AZOTH_INTERFACES_ITRANSFERMANAGER_H
#include <QObject>
#include <QString>

namespace LeechCraft
{
namespace Azoth
{
	enum TransferDirection
	{
		TDIn,
		TDOut
	};

	enum TransferState
	{
		TSOffer,
		TSStarting,
		TSTransfer,
		TSFinished
	};

	enum TransferError
	{
		TENoError,
		TEAborted,
		TEFileAccessError,
		TEFileCorruptError,
		TEProtocolError
	};

	class ITransferJob
	{
	public:
		virtual ~ITransferJob () {}

		virtual QString GetSourceID () const = 0;

		virtual QString GetName () const = 0;

		virtual qint64 GetSize () const = 0;

		virtual void Accept (const QString& out) = 0;

		virtual void Abort () = 0;

		virtual void transferProgress (qint64 done, qint64 total) = 0;

		virtual void errorAppeared (TransferError error, const QString& msg) = 0;

		virtual void stateChanged (TransferState state) = 0;
	};

	class ITransferManager
	{
	public:
		virtual ~ITransferManager () {}

		virtual QObject* SendFile (const QString& id, const QString& name) = 0;

		virtual void fileOffered (QObject*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ITransferJob,
		"org.Deviant.LeechCraft.Azoth.ITransferJob/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Azoth::ITransferManager,
		"org.Deviant.LeechCraft.Azoth.ITransferManager/1.0");

#endif
