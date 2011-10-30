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

#include "scripter.h"
#include <QScriptEngine>
#include <QtDebug>
#include <QStringList>
#include <QFile>
#include <typeinfo>
#include "typeregister.h"
#include "file.h"

using namespace LeechCraft;

Scripter::Scripter (const QDomElement& elem)
: Settings_ (new Settings)
, Container_ (elem)
{
	TypeRegister::Instance ();
}

QStringList Scripter::GetOptions ()
{
	Reset ();
	const QDomElement& valueGenerator = Container_.firstChildElement ("valueGenerator");
	QStringList result;
	if (valueGenerator.isNull ())
	{
		qDebug () << Q_FUNC_INFO << "container has no valueGenerator";
		return result;
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

	result = options.toString ().split (",", QString::KeepEmptyParts);
	return result;
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
	QScriptValue global = Engine_->globalObject ();
	const QStringList& classes = global.property ("RequiredClasses").call ()
		.toString ().split (" ", QString::SkipEmptyParts);

	Q_FOREACH (const QString& elm, classes)
		global.setProperty (elm, TypeRegister::Instance ().GetValueForName (elm,
						Engine_.get ()));

	global.setProperty ("Settings", Engine_->newQObject (Settings_.get ()));
	qScriptRegisterMetaType (Engine_.get (), toScriptValue, fromScriptValue);
}

void Scripter::Reset ()
{
	Engine_.reset (new QScriptEngine);
	Engine_->setProcessEventsInterval (10);
}

