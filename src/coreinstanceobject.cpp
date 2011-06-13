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

#include "coreinstanceobject.h"
#include <QIcon>

namespace LeechCraft
{
	CoreInstanceObject::CoreInstanceObject (QObject *parent)
	: QObject (parent)
	{
	}
	
	void CoreInstanceObject::Init (ICoreProxy_ptr proxy)
	{
	}
	
	void CoreInstanceObject::SecondInit ()
	{
	}
	
	void CoreInstanceObject::Release ()
	{
	}

	QByteArray CoreInstanceObject::GetUniqueID () const
	{
		return "org.LeechCraft.CoreInstance";
	}
	
	QString CoreInstanceObject::GetName () const
	{
		return "LCCore";
	}
	
	QString CoreInstanceObject::GetInfo () const
	{
		return tr ("LeechCraft Core module.");
	}
	
	QIcon CoreInstanceObject::GetIcon () const
	{
		return QIcon (":/resources/images/leechcraft.svg");
	}
}
