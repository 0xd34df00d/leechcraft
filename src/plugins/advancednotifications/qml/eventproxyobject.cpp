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

#include "eventproxyobject.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	EventProxyObject::EventProxyObject (const EventData& ed, QObject *parent)
	: QObject (parent)
	, E_ (ed)
	{
	}

	int EventProxyObject::count () const
	{
		return E_.Count_;
	}
	
	QPixmap EventProxyObject::image () const
	{
		return E_.Pixmap_;
	}
	
	QString EventProxyObject::extendedText () const
	{
		return E_.ExtendedText_;
	}
}
}
