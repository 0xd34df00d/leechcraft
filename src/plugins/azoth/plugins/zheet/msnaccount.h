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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_MSNACCOUNT_H
#include <QObject>
#include <interfaces/iaccount.h>

namespace MSN
{
	class NotificationServerConnection;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNProtocol;
	class Callbacks;

	class MSNAccount : public QObject
					 , public IAccount
	{
		Q_INTERFACES (LeechCraft::Azoth::IAccount);

		MSNProtocol *Proto_;

		Callbacks *CB_;
		MSN::NotificationServerConnection *Conn_;
	public:
		MSNAccount (MSNProtocol* = 0);
	};
}
}
}

#endif
