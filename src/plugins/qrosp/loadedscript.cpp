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

#include "loadedscript.h"
#include <qross/core/action.h>

namespace LeechCraft
{
namespace Qrosp
{
	LoadedScript::LoadedScript (const QString& path,
			const QString& interp, QObject *parent)
	: QObject (parent)
	, ScriptAction_ (new Qross::Action (this, QUrl::fromLocalFile (path)))
	{
		if (!interp.isEmpty ())
			ScriptAction_->setInterpreter (interp);

		ScriptAction_->trigger ();
	}
	
	QVariant LoadedScript::InvokeMethod (const QString& name, const QVariantList& args) const
	{
		if (!ScriptAction_->functionNames ().contains (name))
			return QVariant ();

		return ScriptAction_->callFunction (name, args);
	}
}
}
