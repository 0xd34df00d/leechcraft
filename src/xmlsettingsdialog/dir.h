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

/*
	Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *																		 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	 *
 *   (at your option) any later version.								   *
 *																		 *
 ***************************************************************************
*/
#ifndef XMLSETTINGSDIALOG_DIR_H
#define XMLSETTINGSDIALOG_DIR_H
#include <memory>
#include <QObject>
#include <QMetaType>
#include <QScriptEngine>
#include <QDir>

namespace LeechCraft
{
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
				QDir::Filters = QDir::NoFilter,
				QDir::SortFlags = QDir::NoSort) const;
		QFileInfoList entryInfoList (QDir::Filters = QDir::NoFilter,
				QDir::SortFlags = QDir::NoSort) const;
		QStringList entryList (const QStringList&,
				QDir::Filters = QDir::NoFilter,
				QDir::SortFlags = QDir::NoSort) const;
		QStringList entryList (QDir::Filters = QDir::NoFilter,
				QDir::SortFlags = QDir::NoSort) const;
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
		void setFilter (QDir::Filters);
		void setNameFilters (const QStringList&);
		void setPath (const QString&);
		void setSorting (QDir::SortFlags);
		QDir::SortFlags sorting () const;
	public:
		Q_INVOKABLE bool operator!= (const Dir&) const;
		Q_INVOKABLE Dir& operator= (const Dir&);
		Q_INVOKABLE bool operator== (const Dir&) const;
		Q_INVOKABLE QString operator[] (int) const;
	};
};

Q_DECLARE_METATYPE (LeechCraft::Dir);
Q_SCRIPT_DECLARE_QMETAOBJECT (LeechCraft::Dir, QObject*);

#endif

