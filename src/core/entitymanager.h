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

#pragma once

#include <QObject>
#include "interfaces/core/ientitymanager.h"

namespace LeechCraft
{
	class EntityManager : public QObject
						, public IEntityManager
	{
		Q_OBJECT
		Q_INTERFACES (IEntityManager)
	public:
		EntityManager (QObject* = 0);

		DelegationResult DelegateEntity (Entity, QObject* = 0);
		bool HandleEntity (Entity, QObject* = 0);
		bool CouldHandle (const Entity&);
		QList<QObject*> GetPossibleHandlers (const Entity&);
	};
}
