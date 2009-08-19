/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "typeregister.h"
#include <QMetaType>
#include <QScriptValue>
#include <QtDebug>
#include "dir.h"
#include "file.h"
#include "bytearray.h"

using namespace LeechCraft;

TypeRegister::TypeRegister ()
{
	qRegisterMetaType<Dir> ("Dir");
	qRegisterMetaType<File> ("File");
	qRegisterMetaType<ByteArray> ("ByteArray");
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
		return e->scriptValueFromQMetaObject<File> ();
	else if (name == "ByteArray")
		return e->scriptValueFromQMetaObject<ByteArray> ();
	else
		return QScriptValue ();
}

