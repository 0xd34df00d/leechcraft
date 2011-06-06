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

#ifndef PLUGINS_AZOTH_INTERFACES_IADVANCEDMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IADVANCEDMESSAGE_H
#include <QtPlugin>

namespace LeechCraft
{
namespace Azoth
{
	class IAdvancedMessage
	{
	public:
		virtual ~IAdvancedMessage () {}
		
		virtual bool IsDelivered () const = 0;
		
		virtual void messageDelivered () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IAdvancedMessage,
	"org.Deviant.LeechCraft.Azoth.IAdvancedMessage/1.0");

#endif
