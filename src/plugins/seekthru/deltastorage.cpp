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

#include "deltastorage.h"
#include <stdexcept>
#include <algorithm>
#include <QFile>
#include <QVector>
#include <plugininterface/util.h>
#include <plugininterface/syncops.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			DeltaStorage::DeltaStorage (const QString& id, QObject *parent)
			: QObject (parent)
			, ID_ (id)
			{
			}

			void DeltaStorage::Store (const Sync::ChainID_t& chainId, const Sync::Payload& payload)
			{
				QDir dir = GetDir (chainId);
				int curNum = GetLastFileNum (chainId) + 1;

				StoreImpl (dir.absoluteFilePath (QString::number (curNum)), payload);

				SetLastFileNum (chainId, curNum);
			}

			void DeltaStorage::Store (const Sync::ChainID_t& chainId, const Sync::Payloads_t& payloads)
			{
				QDir dir = GetDir (chainId);
				int curNum = GetLastFileNum (chainId);

				Q_FOREACH (const Sync::Payload& payload, payloads)
					StoreImpl (dir.absoluteFilePath (QString::number (++curNum)), payload);

				SetLastFileNum (chainId, curNum);
			}

			Sync::Payloads_t DeltaStorage::Get (const Sync::ChainID_t& chainId) const
			{
				QMap<int, Sync::Payload> tmpPayloads;

				QDir dir = GetDir (chainId);

				Q_FOREACH (const QString& filename, dir.entryList (QDir::Files | QDir::NoDotAndDotDot))
				{
					bool ok = true;
					int num = filename.toInt (&ok);
					if (!ok)
						continue;

					QFile file (dir.absoluteFilePath (filename));
					if (!file.open (QIODevice::ReadOnly))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to open"
								<< file.fileName ()
								<< "for reading:"
								<< file.errorString ();
						throw std::runtime_error ("Unable to open file for reading.");
					}

					QByteArray data = file.readAll ();
					Sync::Payload payload = Sync::Deserialize (qUncompress (data));
					tmpPayloads [num] = payload;
				}

				QList<Sync::Payload> result;
				QList<int> keys = tmpPayloads.keys ();
				std::sort (keys.begin (), keys.end ());
				Q_FOREACH (int key, keys)
					result << tmpPayloads [key];

				return result;
			}

			void DeltaStorage::Purge (const Sync::ChainID_t& chainId, quint32 num)
			{
				QDir dir = GetDir (chainId);
				quint32 purged = 0;
				Q_FOREACH (const QString& filename, dir.entryList (QDir::Files | QDir::NoDotAndDotDot))
				{
					if (!dir.remove (filename))
						qWarning () << Q_FUNC_INFO
								<< "could not remove"
								<< filename;
					if (++purged == num)
						break;
				}
			}

			QDir DeltaStorage::GetDir (const Sync::ChainID_t& chainId) const
			{
				return Util::CreateIfNotExists ("deltastorage/" + ID_ + "/" + chainId);
			}

			int DeltaStorage::GetLastFileNum (const Sync::ChainID_t& chainId) const
			{
				QDir dir = GetDir (chainId);

				if (!dir.exists ("seq"))
				{
					SetLastFileNum (chainId, 0);
					return 0;
				}

				QString fileName = dir.absoluteFilePath ("seq");
				QFile file (fileName);
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to open file"
							<< file.fileName ()
							<< "for reading:"
							<< file.errorString ();
					throw std::runtime_error ("Unable to open file for reading.");
				}

				return file.readAll ().toInt ();
			}

			void DeltaStorage::SetLastFileNum (const Sync::ChainID_t& chainId, int num) const
			{
				QString fileName = GetDir (chainId).absoluteFilePath ("seq");

				QFile file (fileName);
				if (!file.open (QIODevice::WriteOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to open file"
							<< file.fileName ()
							<< "for writing:"
							<< file.errorString ();
					throw std::runtime_error ("Unable to open file for writing.");
				}
				file.write (QByteArray::number (num));
			}

			void DeltaStorage::StoreImpl (const QString& path, const Sync::Payload& payload)
			{
				QFile file (path);
				if (!file.open (QIODevice::WriteOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to open file"
							<< file.fileName ()
							<< "for writing:"
							<< file.errorString ();
					throw std::runtime_error ("Unable to open file for writing.");
				}

				file.write (qCompress (Sync::Serialize (payload), 5));
			}
		}
	}
}
