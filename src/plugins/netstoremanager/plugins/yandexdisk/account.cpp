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

#include "account.h"
#include "yandexdisk.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	Account::Account (Plugin *plugin)
	: QObject (plugin)
	, Plugin_ (plugin)
	{
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	AccountFeatures Account::GetAccountFeatures () const
	{
		return AccountFeature::FileListings | AccountFeature::ProlongateFiles;
	}

	void Account::Upload (const QString& path) const
	{
	}
}
}
}
