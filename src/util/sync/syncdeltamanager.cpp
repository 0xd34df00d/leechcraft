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

#include "syncdeltamanager.h"
#include <stdexcept>
#include <algorithm>
#include <QFile>
#include <QCoreApplication>
#include <util/util.h>
#include <util/sync/syncops.h>

namespace LeechCraft
{
	namespace Util
	{
		SyncDeltaManager::SyncDeltaManager (const QString& id, QObject *parent)
		: QObject (parent)
		, ID_ (id)
		, Settings_ (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ())
		{
			Settings_.beginGroup ("SyncDeltaManager");
			Settings_.beginGroup (id);
		}

		SyncDeltaManager::~SyncDeltaManager ()
		{
			Settings_.endGroup ();
			Settings_.endGroup ();
		}

		void SyncDeltaManager::Store (const Sync::ChainID_t& chainId, const Sync::Payload& payload)
		{
			Settings_.beginGroup (chainId);
			bool shouldStore = Settings_.value ("EverRequested", false).toBool ();
			Settings_.endGroup ();
			if (!shouldStore)
				return;

			QDir dir = GetDir (chainId);
			int curNum = GetLastFileNum (chainId) + 1;

			StoreImpl (dir.absoluteFilePath (QString::number (curNum)), payload);

			SetLastFileNum (chainId, curNum);
		}

		void SyncDeltaManager::Store (const Sync::ChainID_t& chainId, const Sync::Payloads_t& payloads)
		{
			Settings_.beginGroup (chainId);
			bool shouldStore = Settings_.value ("EverRequested", false).toBool ();
			Settings_.endGroup ();
			if (!shouldStore)
				return;

			QDir dir = GetDir (chainId);
			int curNum = GetLastFileNum (chainId);

			Q_FOREACH (const Sync::Payload& payload, payloads)
				StoreImpl (dir.absoluteFilePath (QString::number (++curNum)), payload);

			SetLastFileNum (chainId, curNum);
		}

		Sync::Payloads_t SyncDeltaManager::Get (const Sync::ChainID_t& chainId)
		{
			DeltasRequested (chainId);

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

		void SyncDeltaManager::Purge (const Sync::ChainID_t& chainId, quint32 num)
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

		void SyncDeltaManager::DeltasRequested (const Sync::ChainID_t& chainId)
		{
			Settings_.beginGroup (chainId);
			Settings_.setValue ("EverRequested", true);
			Settings_.endGroup ();
		}

		QDir SyncDeltaManager::GetDir (const Sync::ChainID_t& chainId) const
		{
			return Util::CreateIfNotExists ("deltastorage/" + ID_ + "/" + chainId);
		}

		int SyncDeltaManager::GetLastFileNum (const Sync::ChainID_t& chainId)
		{
			Settings_.beginGroup (chainId);
			int num = Settings_.value ("LastFileNum", 0).toInt ();
			Settings_.endGroup ();
			return num;
		}

		void SyncDeltaManager::SetLastFileNum (const Sync::ChainID_t& chainId, int num)
		{
			Settings_.beginGroup (chainId);
			Settings_.setValue ("LastFileNum", num);
			Settings_.endGroup ();
		}

		void SyncDeltaManager::StoreImpl (const QString& path, const Sync::Payload& payload)
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

