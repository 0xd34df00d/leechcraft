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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ENTRYBASE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ENTRYBASE_H
#include <QObject>
#include <interfaces/iclentry.h>

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
					class GlooxMessage;

					class EntryBase : public QObject
									, public ICLEntry
					{
						Q_OBJECT
					protected:
						QList<QObject*> AllMessages_;
						EntryStatus CurrentStatus_;
						QList<QAction*> Actions_;
					public:
						EntryBase (QObject* = 0);

						virtual QObject* GetObject ();
						virtual QList<QObject*> GetAllMessages () const;
						EntryStatus GetStatus () const;
						QList<QAction*> GetActions ();

						void HandleMessage (GlooxMessage*);
						void SetStatus (const EntryStatus&);
					signals:
						void gotMessage (QObject*);
						void statusChanged (const Plugins::EntryStatus&);
					};
				}
			}
		}
	}
}

#endif
