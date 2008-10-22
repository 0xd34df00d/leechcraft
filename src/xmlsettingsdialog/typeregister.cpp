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
#include "typeregister.h"
#include <QMetaType>
#include <QScriptValue>
#include <QtDebug>
#include "dir.h"
#include "file.h"

TypeRegister::TypeRegister ()
{
	qRegisterMetaType<Dir> ("Dir");
}

TypeRegister& TypeRegister::Instance ()
{
	static TypeRegister obj;
	return obj;
}

QScriptValue TypeRegister::GetValueForName (const QString& name,
		QScriptEngine *e) const
{
	if (name == "Dir")
		return e->scriptValueFromQMetaObject<Dir> ();
	else if (name == "File")
		return e->scriptValueFromQMetaObject<QFile> ();
	else
		return QScriptValue ();
}

