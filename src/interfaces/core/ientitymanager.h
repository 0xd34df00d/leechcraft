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

#pragma once

#include <QObject>

namespace LeechCraft
{
	struct Entity;
}

class IEntityManager
{
public:
	struct DelegationResult
	{
		QObject *Handler_;
		int ID_;
	};

	virtual ~IEntityManager () {}

	virtual DelegationResult DelegateEntity (LeechCraft::Entity, QObject *desired = 0) = 0;

	virtual bool HandleEntity (LeechCraft::Entity, QObject *desired = 0) = 0;

	virtual bool CouldHandle (const LeechCraft::Entity&) = 0;

	virtual QList<QObject*> GetPossibleHandlers (const LeechCraft::Entity&) = 0;
};

Q_DECLARE_INTERFACE (IEntityManager, "org.Deviant.LeechCraft.IEntityManager/1.0");
