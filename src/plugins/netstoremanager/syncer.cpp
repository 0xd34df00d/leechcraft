/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "syncer.h"
#include <QtDebug>
#include "interfaces/netstoremanager/istorageaccount.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	Syncer::Syncer (const QString& dirPath, IStorageAccount *isa, QObject *parent)
	: QObject (parent)
	, BasePath_ (dirPath)
	, Account_ (isa)
	{
	}

	QByteArray Syncer::GetAccountID () const
	{
		return Account_->GetUniqueID ();
	}

	QString Syncer::GetBasePath () const
	{
		return BasePath_;
	}

	void Syncer::start ()
	{
	}

	void Syncer::stop ()
	{
	}

	void Syncer::dirWasCreated (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::dirWasRemoved (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::fileWasCreated (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::fileWasRemoved (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::fileWasUpdated (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

}
}
