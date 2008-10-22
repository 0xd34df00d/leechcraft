/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
#include "scripter.h"
#include <QScriptEngine>
#include <QtDebug>
#include <QStringList>
#include <QFile>
#include <typeinfo>
#include "typeregister.h"

Scripter::Scripter (const QDomElement& elem)
: Container_ (elem)
{
	TypeRegister::Instance ();
}

QStringList Scripter::GetOptions ()
{
	Reset ();
	QDomElement valueGenerator = Container_.firstChildElement ("valueGenerator");
	QStringList result;
	if (valueGenerator.isNull ())
	{
		qDebug () << Q_FUNC_INFO << "container has no valueGenerator";
		return result;
	}
	QString script = GetScript (valueGenerator);

	QString str = Engine_->evaluate (script).toString ();
	if (Engine_->hasUncaughtException ())
	{
		qWarning () << Q_FUNC_INFO
			<< "script exception occured on line"
			<< Engine_->uncaughtExceptionLineNumber ()
			<< str;
		return QStringList ();
	}

	FeedRequiredClasses ();

	QScriptValue global = Engine_->globalObject ();

	QVariant options = global.property ("GetOptions").call ().toVariant ();

	result = options.toString ().split (",", QString::KeepEmptyParts);
	return result;
}


QString Scripter::HumanReadableOption (const QString& name)
{
	QScriptValue global = Engine_->globalObject ();

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
		QString fileName = elem.text ();
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
	QScriptValue global = Engine_->globalObject ();
	QStringList classes = global.property ("RequiredClasses").call ()
		.toString ().split (" ", QString::SkipEmptyParts);

	for (QStringList::const_iterator i = classes.begin (),
			end = classes.end (); i != end; ++i)
		global.setProperty (*i,
				TypeRegister::Instance ().GetValueForName (*i,
					Engine_.get ()));
}

void Scripter::Reset ()
{
	Engine_.reset (new QScriptEngine);
	Engine_->setProcessEventsInterval (10);
}

