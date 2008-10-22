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
#include <memory>
#include <QObject>
#include <QMetaType>
#include <QScriptEngine>
#include <QDir>

class Dir : public QObject
{
	Q_OBJECT

	std::auto_ptr<QDir> Imp_;
public:
	Dir (QObject* = 0);
	Dir (const Dir&);
	virtual ~Dir ();
public slots:
	QString absoluteFilePath (const QString&) const;
	QString absolutePath () const;
	QString canonicalPath () const;
	bool cd (const QString&);
	bool cdUp ();
	uint count () const;
	QString dirName () const;
	QFileInfoList entryInfoList (const QStringList&,
			QDir::Filters,
			QDir::SortFlags) const;
	QFileInfoList entryInfoList (QDir::Filters,
			QDir::SortFlags) const;
	QStringList entryList (const QStringList&,
			QDir::Filters,
			QDir::SortFlags) const;
	QStringList entryList (QDir::Filters,
			QDir::SortFlags) const;
	bool exists (const QString&) const;
	bool exists () const;
	QString filePath (const QString&) const;
	QDir::Filters filter () const;
	bool isAbsolute () const;
	bool isReadable () const;
	bool isRelative () const;
	bool isRoot () const;
	bool makeAbsolute ();
	bool mkdir (const QString&) const;
	bool mkpath (const QString&) const;
	QStringList nameFilters () const;
	QString path () const;
	void refresh () const;
	QString relativeFilePath (const QString&) const;
	bool remove (const QString&);
	bool rename (const QString&, const QString&);
	bool rmdir (const QString&) const;
	bool rmpath (const QString&) const;
};

Q_DECLARE_METATYPE (Dir);
Q_SCRIPT_DECLARE_QMETAOBJECT (Dir, QObject*);

#endif

