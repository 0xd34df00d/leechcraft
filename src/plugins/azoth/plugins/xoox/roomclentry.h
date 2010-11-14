/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#include <QObject>
#include <QStringList>
#include <interfaces/iclentry.h>

namespace gloox
{
	class MUCRoom;
}

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace Xoox
				{
					class GlooxAccount;
					class RoomPublicMessage;

					class RoomCLEntry : public QObject
									  , public ICLEntry
					{
						Q_OBJECT
						Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::ICLEntry);

						GlooxAccount *Account_;
						gloox::MUCRoom *Room_;
						QList<IMessage*> AllMessages_;
					public:
						RoomCLEntry (gloox::MUCRoom*, GlooxAccount*);

						QObject* GetObject ();
						IAccount* GetParentAccount () const ;
						Features GetEntryFeatures () const;
						QString GetEntryName () const;
						void SetEntryName (const QString&);
						QByteArray GetEntryID () const;
						QStringList Groups () const;
						QStringList Variants () const;
						IMessage* CreateMessage (IMessage::MessageType,
								const QString&, const QString&);
						QList<IMessage*> GetAllMessages () const;
						EntryStatus GetStatus () const;

						gloox::MUCRoom* GetRoom ();

						void HandleMessage (RoomPublicMessage*);
					signals:
						void gotMessage (QObject*);
						void statusChanged (const EntryStatus&);
						void availableVariantsChanged (const QStringList&);
					};
				}
			}
		}
	}
}

#endif
