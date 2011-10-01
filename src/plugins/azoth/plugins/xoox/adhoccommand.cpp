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

#include "adhoccommand.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	AdHocCommand::AdHocCommand (const QString& name, const QString& node)
	: Name_ (name)
	, Node_ (node)
	{
	}

	QString AdHocCommand::GetName () const
	{
		return Name_;
	}

	void AdHocCommand::SetName (const QString& name)
	{
		Name_ = name;
	}

	QString AdHocCommand::GetNode () const
	{
		return Node_;
	}

	void AdHocCommand::SetNode (const QString& node)
	{
		Node_ = node;
	}

	QString AdHocResult::GetNode () const
	{
		return Node_;
	}

	void AdHocResult::SetNode (const QString& node)
	{
		Node_ = node;
	}

	QString AdHocResult::GetSessionID () const
	{
		return SessionID_;
	}

	void AdHocResult::SetSessionID (const QString& sid)
	{
		SessionID_ = sid;
	}

	QXmppDataForm AdHocResult::GetDataForm () const
	{
		return Form_;
	}

	void AdHocResult::SetDataForm (const QXmppDataForm& form)
	{
		Form_ = form;
	}

	QStringList AdHocResult::GetActions () const
	{
		return Actions_;
	}

	void AdHocResult::SetActions (const QStringList& actions)
	{
		Actions_ = actions;
	}
}
}
}
