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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#define PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#include <QMetaType>

namespace LeechCraft
{
namespace Azoth
{
	class IHaveConsole
	{
	public:
		virtual ~IHaveConsole () {}
		
		enum PacketFormat
		{
			PFXML,
			PFPlainText,
			PFBinary
		};
		
		enum PacketDirection
		{
			PDIn,
			PDOut
		};

		virtual PacketFormat GetPacketFormat () const = 0;
		
		virtual void SetConsoleEnabled (bool) = 0;
		
		virtual void gotConsolePacket (const QByteArray& packet, int direction) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveConsole,
		"org.Deviant.LeechCraft.Azoth.IHaveConsole/1.0");

#endif
