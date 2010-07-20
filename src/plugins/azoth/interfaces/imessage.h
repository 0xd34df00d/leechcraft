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

#ifndef PLUGINS_AZOTH_INTERFACES_IMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IMESSAGE_H
#include <QString>
#include <QDateTime>

class QObject;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class ICLEntry;

				class IMessage
				{
				public:
					virtual ~IMessage () {};

					enum Direction
					{
						DIn,
						DOut
					};

					enum MessageType
					{
						MTChat,
						MTGroupchat
					};

					virtual QObject* GetObject () = 0;
					virtual void Send () = 0;
					virtual Direction GetDirection () const = 0;
					virtual MessageType GetMessageType () const = 0;
					virtual ICLEntry* OtherPart () const = 0;
					virtual QString GetOtherVariant () const = 0;
					virtual QString GetBody () const = 0;
					virtual void SetBody (const QString&) = 0;
				};
			}
		}
	}
}

#endif
