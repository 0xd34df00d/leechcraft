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
#ifndef FILE_H
#define FILE_H
#include <QFile>
#include <QScriptEngine>

class File : public QFile
{
	Q_OBJECT
public:
	File (QObject* = 0);
	File (const File&);
	virtual ~File ();
public slots:
};

Q_DECLARE_METATYPE (Dir);
Q_SCRIPT_DECLARE_QMETAOBJECT (Dir, QObject*);

#endif

