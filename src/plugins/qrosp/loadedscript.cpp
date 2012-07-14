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

#include "loadedscript.h"
#ifndef QROSP_NO_QTSCRIPT
#include <QScriptEngine>
#endif
#include <QtDebug>
#include <qross/core/action.h>
#include <qross/core/script.h>

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
#ifndef QROSP_NO_QTSCRIPT
		if (interp == "qtscript")
		{
			QObject *scriptEngineObject = 0;
			QMetaObject::invokeMethod (ScriptAction_->script (),
					"engine", Q_RETURN_ARG (QObject*, scriptEngineObject));
			QScriptEngine *engine = qobject_cast<QScriptEngine*> (scriptEngineObject);
			if (!engine)
				qWarning () << Q_FUNC_INFO
						<< "unable to obtain script engine from the"
						<< "script action though we are Qt Script";
			else
			{
				QStringList requires;
				requires << "qt" << "qt.core" << "qt.gui" << "qt.network" << "qt.webkit";
				Q_FOREACH (const QString& req, requires)
					engine->importExtension (req);
			}
		}
#endif
	}
	
	QVariant LoadedScript::InvokeMethod (const QString& name, const QVariantList& args) const
	{
		if (!ScriptAction_->functionNames ().contains (name))
			return QVariant ();

		return ScriptAction_->callFunction (name, args);
	}
}
}
