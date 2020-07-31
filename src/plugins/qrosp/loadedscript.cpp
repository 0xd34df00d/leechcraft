/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loadedscript.h"
#ifndef QROSP_NO_QTSCRIPT
#include <QScriptEngine>
#endif
#include <QtDebug>
#include <qross/core/action.h>
#include <qross/core/script.h>
#include "lcenv.h"

namespace LC
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
	}

	QObject* LoadedScript::GetQObject ()
	{
		return this;
	}

	QVariant LoadedScript::InvokeMethod (const QString& name, const QVariantList& args)
	{
		if (!Imported_)
			CheckImports ();

		if (!ScriptAction_->functionNames ().contains (name))
		{
			qWarning () << Q_FUNC_INFO
					<< "no such function"
					<< name
					<< "in"
					<< ScriptAction_->file ();
			qWarning () << Q_FUNC_INFO
					<< "known function:"
					<< ScriptAction_->functionNames ();
			return {};
		}

		return ScriptAction_->callFunction (name, args);
	}

	void LoadedScript::AddQObject (QObject *object, const QString& name)
	{
		ScriptAction_->addQObject (object, name);
	}

	void LoadedScript::Execute ()
	{
		if (!Imported_)
			CheckImports ();

		ScriptAction_->trigger ();
	}

	void LoadedScript::CheckImports ()
	{
		if (Imported_)
			return;

		ScriptAction_->addQObject (new LCEnv (this), "LCEnv");

		Imported_ = true;
		ScriptAction_->trigger ();

#ifndef QROSP_NO_QTSCRIPT
		if (ScriptAction_->interpreter () != "qtscript")
			return;

		QObject *scriptEngineObject = nullptr;
		QMetaObject::invokeMethod (ScriptAction_->script (),
				"engine",
				Q_RETURN_ARG (QObject*, scriptEngineObject));
		auto engine = qobject_cast<QScriptEngine*> (scriptEngineObject);
		if (!engine)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to obtain script engine from the"
					<< "script action though we are Qt Script";
			return;
		}
#endif
	}
}
}
