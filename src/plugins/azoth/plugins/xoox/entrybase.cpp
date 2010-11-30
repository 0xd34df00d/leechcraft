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

#include "entrybase.h"
#include "glooxmessage.h"

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
					EntryBase::EntryBase (QObject* parent)
					: QObject (parent)
					{

					}

					QObject* EntryBase::GetObject ()
					{
						return this;
					}


					QList<QObject*> EntryBase::GetAllMessages () const
					{
						return AllMessages_;
					}

					EntryStatus EntryBase::GetStatus () const
					{
						return CurrentStatus_;
					}

					QList<QAction*> EntryBase::GetActions ()
					{
						return Actions_;
					}

					void EntryBase::HandleMessage (GlooxMessage *msg)
					{
						AllMessages_ << msg;
						emit gotMessage (msg);
					}

					void EntryBase::SetStatus (const EntryStatus& status)
					{
						if (status == CurrentStatus_)
							return;

						CurrentStatus_ = status;
						emit statusChanged (CurrentStatus_);
					}
				}
			}
		}
	}
}
