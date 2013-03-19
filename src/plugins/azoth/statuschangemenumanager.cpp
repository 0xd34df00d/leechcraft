/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "statuschangemenumanager.h"
#include <QMenu>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	StatusChangeMenuManager::StatusChangeMenuManager (QObject *parent)
	: QObject (parent)
	{
	}

	QMenu* StatusChangeMenuManager::CreateMenu (QObject* obj, const char* slot, QWidget *parent)
	{
		QMenu *result = new QMenu (tr ("Change status"), parent);
		result->addAction (Core::Instance ().GetIconForState (SOnline),
				tr ("Online"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOnline));
		result ->addAction (Core::Instance ().GetIconForState (SChat),
				tr ("Free to chat"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SChat));
		result ->addAction (Core::Instance ().GetIconForState (SAway),
				tr ("Away"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SAway));
		result ->addAction (Core::Instance ().GetIconForState (SDND),
				tr ("DND"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SDND));
		result ->addAction (Core::Instance ().GetIconForState (SXA),
				tr ("Not available"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SXA));
		result ->addAction (Core::Instance ().GetIconForState (SOffline),
				tr ("Offline"), obj, slot)->
					setProperty ("Azoth/TargetState",
							QVariant::fromValue<State> (SOffline));

		result->addSeparator ();
		result->addAction (tr ("Custom..."), obj, slot);

		return result;
	}
}
}
