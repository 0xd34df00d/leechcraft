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
#ifndef XMLSETTINGSDIALOG_FILE_H
#define XMLSETTINGSDIALOG_FILE_H
#include <memory>
#include <QScriptEngine>
#include <QFile>
#include "bytearray.h"

namespace LeechCraft
{
	class File : public QObject
	{
		Q_OBJECT

		std::auto_ptr<QFile> Imp_;
	public:
		File (QObject* = 0);
		File (const File&);
		virtual ~File ();
	public slots:
		bool atEnd () const;
		qint64 bytesAvailable () const;
		qint64 bytesToWrite () const;
		bool canReadLine () const;
		void close ();
		bool copy (const QString&);
		QFile::FileError error () const;
		QString errorString () const;
		bool exists () const;
		QString fileName () const;
		bool flush ();
		int handle () const;
		bool isOpen () const;
		bool isReadable () const;
		bool isSequential () const;
		bool isTextModeEnabled () const;
		bool isWritable () const;
		bool link (const QString&);
		bool open (QIODevice::OpenMode = QIODevice::ReadOnly);
		QFile::OpenMode openMode () const;
		ByteArray peek (qint64);
		QFile::Permissions permissions () const;
		qint64 pos () const;
		bool putChar (char);
		ByteArray read (qint64);
		ByteArray readAll ();
		ByteArray readLine (qint64);
		bool remove ();
		bool rename (const QString&);
		bool reset ();
		bool resize (qint64);
		bool seek (qint64);
		void setFileName (const QString&);
		bool setPermissions (QFile::Permissions);
		void setTextModeEnabled (bool);
		qint64 size () const;
		QString symLinkTarget () const;
		void unsetError ();
		bool waitForBytesWritten (int);
		bool waitForReadyRead (int);
		qint64 write (const ByteArray&);
	};
};

Q_DECLARE_METATYPE (LeechCraft::File);
Q_DECLARE_METATYPE (QIODevice::OpenMode);
Q_SCRIPT_DECLARE_QMETAOBJECT (LeechCraft::File, QObject*);

QScriptValue toScriptValue (QScriptEngine*, const QIODevice::OpenMode&);
void fromScriptValue (const QScriptValue&, QIODevice::OpenMode&);

#endif

