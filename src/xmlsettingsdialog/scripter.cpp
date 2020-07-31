/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scripter.h"
#include <QScriptEngine>
#include <QtDebug>
#include <QStringList>
#include <QFile>

using namespace LC;

Scripter::Scripter (const QDomElement& elem)
: Settings_ (new Settings)
, Container_ (elem)
{
}

QStringList Scripter::GetOptions ()
{
	Reset ();
	const QDomElement& valueGenerator = Container_.firstChildElement ("valueGenerator");
	if (valueGenerator.isNull ())
	{
		qDebug () << Q_FUNC_INFO << "container has no valueGenerator";
		return {};
	}
	const QString& script = GetScript (valueGenerator);

	const QString& str = Engine_->evaluate (script).toString ();
	if (Engine_->hasUncaughtException ())
	{
		qWarning () << Q_FUNC_INFO
			<< "script exception occured on line"
			<< Engine_->uncaughtExceptionLineNumber ()
			<< str;
		return QStringList ();
	}

	FeedRequiredClasses ();

	const QScriptValue& global = Engine_->globalObject ();
	const QVariant& options = global.property ("GetOptions").call ().toVariant ();
	return options.toString ().split (",");
}


QString Scripter::HumanReadableOption (const QString& name)
{
	const QScriptValue& global = Engine_->globalObject ();

	QScriptValueList args;
	args << QScriptValue (Engine_.get (), name);

	return global.property ("OptionValueToName")
		.call (QScriptValue (), args).toString ();
}

QString Scripter::GetScript (const QDomElement& elem) const
{
	QString programText;
	if (elem.attribute ("place") == "file")
	{
		const QString& fileName = elem.text ();
		QFile file (fileName);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "Could not load script file"
				<< fileName;
			return programText;
		}
		programText = file.readAll ();
	}
	else
	{
		qWarning () << Q_FUNC_INFO
			<< "unknown script container"
			<< elem.attribute ("place");
		return programText;
	}
	return programText;
}

void Scripter::FeedRequiredClasses () const
{
	Engine_->importExtension ("qt.core");
}

void Scripter::Reset ()
{
	Engine_.reset (new QScriptEngine);
	Engine_->setProcessEventsInterval (10);
}

