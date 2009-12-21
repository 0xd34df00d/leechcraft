/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "wrapperobject.h"
#include <QIcon>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			void* WrapperObject::qt_metacast (const char *_clname)
			{
				if (!_clname) return 0;
				if (!strcmp(_clname, "IInfo") &&
						Implements (_clname))
					return static_cast< IInfo*>(const_cast< WrapperObject*>(this));
				if (!strcmp(_clname, "org.Deviant.LeechCraft.IInfo/1.0") &&
						Implements (_clname))
					return static_cast< IInfo*>(const_cast< WrapperObject*>(this));
				return QObject::qt_metacast(_clname);
			}

			WrapperObject::WrapperObject (QObject *parent)
			: QObject (parent)
			{
			}

			void WrapperObject::Init (ICoreProxy_ptr)
			{
			}

			void WrapperObject::SecondInit ()
			{
			}

			void WrapperObject::Release ()
			{
			}

			QString WrapperObject::GetName () const
			{
				return QString ();
			}

			QString WrapperObject::GetInfo () const
			{
				return QString ();
			}

			QIcon WrapperObject::GetIcon () const
			{
				return QIcon ();
			}

			QStringList WrapperObject::Provides () const
			{
				return QStringList ();
			}

			QStringList WrapperObject::Needs () const
			{
				return QStringList ();
			}

			QStringList WrapperObject::Uses () const
			{
				return QStringList ();
			}

			void WrapperObject::SetProvider (QObject*, const QString&)
			{
			}

			bool WrapperObject::Implements (const char *interface)
			{
				return true;
			}
		};
	};
};

