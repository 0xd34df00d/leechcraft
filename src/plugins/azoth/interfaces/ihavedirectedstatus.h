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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVEDIRECTEDSTATUS_H
#define PLUGINS_AZOTH_INTERFACES_IHAVEDIRECTEDSTATUS_H
#include <QtPlugin>
#include "iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	class IHaveDirectedStatus
	{
	public:
		virtual ~IHaveDirectedStatus () {}

		virtual bool CanSendDirectedStatusNow (const QString&) = 0;

		virtual void SendDirectedStatus (const EntryStatus& status, const QString&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveDirectedStatus,
		"org.Deviant.LeechCraft.Azoth.IHaveDirectedStatus/1.0");

#endif
