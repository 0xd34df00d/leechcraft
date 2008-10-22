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
#ifndef DIR_H
#define DIR_H
#include <QDir>
#include <QObject>
#include <QMetaType>
#include <QScriptEngine>

class Dir : public QObject
		  , public QDir
{
	Q_OBJECT
public:
	Dir (QObject* = 0);
	Dir (const Dir&);
	virtual ~Dir ();
};

Q_DECLARE_METATYPE (Dir);
Q_SCRIPT_DECLARE_QMETAOBJECT (Dir, QObject*);

#endif

