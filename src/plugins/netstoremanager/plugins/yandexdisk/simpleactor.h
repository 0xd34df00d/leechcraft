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

#ifndef PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_SIMPLEACTOR_H
#define PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_SIMPLEACTOR_H
#include <QNetworkCookie>
#include <interfaces/netstoremanager/isupportfilelistings.h>
#include "actorbase.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	class Account;

	class SimpleActor : public ActorBase
	{
		QUrl URL_;
		QByteArray Post_;
	public:
		SimpleActor (const QUrl&, const QByteArray&, Account*);
	protected:
		QNetworkReply* MakeRequest ();
		void HandleReply (QNetworkReply*);
	};
}
}
}

#endif
