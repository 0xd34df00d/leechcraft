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

#include "eventproxyobject.h"
#include <QVariant>
#include <QDeclarativeContext>
#include <util/util.h>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	EventProxyObject::EventProxyObject (const EventData& ed, QObject *parent)
	: QObject (parent)
	, E_ (ed)
	{
		CachedImage_ = QUrl (Util::GetAsBase64Src (E_.Pixmap_.scaled (32, 32).toImage ()));
		ActionsModel_ = QVariant::fromValue (ed.Actions_);
	}

	int EventProxyObject::count () const
	{
		return E_.Count_;
	}

	QUrl EventProxyObject::image () const
	{
		return CachedImage_;
	}

	QString EventProxyObject::extendedText () const
	{
		return E_.FullText_.isEmpty () ?
				E_.ExtendedText_ :
				E_.FullText_;
	}

	QString EventProxyObject::eventID () const
	{
		return E_.EventID_;
	}

	QVariant EventProxyObject::eventActionsModel () const
	{
		return ActionsModel_;
	}
}
}
