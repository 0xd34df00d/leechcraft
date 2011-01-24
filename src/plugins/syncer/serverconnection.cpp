/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "serverconnection.h"
#include <QTcpSocket>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			ServerConnection::ServerConnection (const QByteArray& chain, QObject *parent)
			: QObject (parent)
			, Socket_ (new QTcpSocket (this))
			, Chain_ (chain)
			{
				connect (Socket_,
						SIGNAL (connected ()),
						this,
						SLOT (handleConnected ()));
				connect (Socket_,
						SIGNAL (readyRead ()),
						this,
						SLOT (handleReadyRead ()));
			}

			QByteArray ServerConnection::FmtMsg (const QList<QByteArray>& lists)
			{
				QByteArray msg;
				{
					QDataStream ds (&msg, QIODevice::WriteOnly);
					quint32 len = 4;
					Q_FOREACH (const QByteArray& ba, lists)
						len += ba.size () + 4;
					ds << len;
					ds << static_cast<quint32> (lists.size ());
					Q_FOREACH (const QByteArray& ba, lists)
						ds << static_cast<quint32> (ba.size ());
					Q_FOREACH (const QByteArray& ba, lists)
						for (int i = 0, size = ba.size (); i < size; ++i)
							ds << static_cast<unsigned char> (ba.at (i));
				}
				return msg;
			}

			QList<QByteArray> ServerConnection::UnfmtMsg (const QByteArray& msg)
			{
				QDataStream ds (msg);
				QList<QByteArray> result;
				quint32 len = 0;
				quint32 numLists = 0;
				ds >> len
					>> numLists;
				QList<quint32> listLengths;

				for (quint32 i = 0; i < numLists; ++i)
				{
					quint32 ll = 0;
					ds >> ll;
					listLengths << ll;
				}

				for (quint32 i = 0; i < numLists; ++i)
				{
					QByteArray list;
					for (quint32 b = 0, numBytes = listLengths.at (i);
							b < numBytes; ++b)
					{
						unsigned char byte = 0;
						ds >> byte;
						list.append (byte);
					}
					result << list;
				}

				return result;
			}

			void ServerConnection::performLogin ()
			{
				if (Socket_->isOpen ())
					Socket_->close ();

				const QString& host = XmlSettingsManager::Instance ()
						.property ("ServerHost").toString ();
				int port = XmlSettingsManager::Instance ()
						.property ("ServerPort").toInt ();
				qDebug () << Q_FUNC_INFO << host << port;
				Socket_->connectToHost (host, port);
			}

			void ServerConnection::reqMaxDelta ()
			{
				QList<QByteArray> lists;
				lists << "MAXDELTA" << Chain_;
				Socket_->write (FmtMsg (lists));
			}

			void ServerConnection::getDeltas (quint32 from)
			{
				QByteArray fromData;
				{
					QDataStream ds (&fromData, QIODevice::WriteOnly);
					ds << from;
				}
				QList<QByteArray> lists;
				lists << "GETDELTA" << Chain_ << fromData;
				Socket_->write (FmtMsg (lists));
			}

			void ServerConnection::putDeltas (const QList<QByteArray>& deltas, quint32 firstId)
			{
				qDebug () << Q_FUNC_INFO << "putting" << deltas.size () << "deltas";
				QList<QByteArray> lists;
				lists << "PUTDELTA" << Chain_;
				QByteArray fiData;
				{
					QDataStream ds (&fiData, QIODevice::WriteOnly);
					ds << firstId;
				}
				lists << fiData;
				lists += deltas;
				Socket_->write (FmtMsg (lists));
			}

			void ServerConnection::handleConnected ()
			{
				const QString& login =
						XmlSettingsManager::Instance ().property ("ServerLogin").toString ();
				const QString& pass =
						XmlSettingsManager::Instance ().property ("ServerPass").toString ();
				qDebug () << Q_FUNC_INFO << login;
				QList<QByteArray> lists;
				lists << "LOGIN" << login.toUtf8 () << pass.toUtf8 ();
				Socket_->write (FmtMsg (lists));
			}

			void ServerConnection::handleReadyRead ()
			{
				qDebug () << Q_FUNC_INFO << Socket_->bytesAvailable ();
				if (Socket_->bytesAvailable () < 4)
					return;

				// Read first four bytes and check if we have at
				// least that much of data.
				QDataStream ds (Socket_->peek (4));
				quint32 size = 0;
				ds >> size;
				size += 4;
				qDebug () << "size:" << size;
				if (Socket_->bytesAvailable () < size)
					return;

				QList<QByteArray> lists = UnfmtMsg (Socket_->read (size));
				qDebug () << "received:";
				Q_FOREACH (const QByteArray& list, lists)
					qDebug () << list.toHex ();
				if (!lists.size ())
				{
					qWarning () << Q_FUNC_INFO
							<< "empty lists received";
					emit fail ();
					return;
				}

				if (lists.at (0) == "OK")
				{
					lists.takeFirst ();
					emit success (lists);
				}
				else if (lists.at (0) == "ERR")
				{
					quint32 code = -1;
					if (lists.size () >= 2 &&
							lists.at (1).size () == 4)
					{
						QByteArray l = lists.at (1);
						code = (l.at (0) << 24) + (l.at (1) << 16) + (l.at (2) << 8) + l.at (3);
					}

					switch (code)
					{
					case ECWrongDeltaID:
						emit deltaOutOfOrder ();
						break;
					default:
						emit fail ();
						break;
					}
				}
			}
		}
	}
}
