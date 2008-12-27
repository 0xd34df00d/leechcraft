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
#include "file.h"

using namespace LeechCraft;

File::File (QObject *parent)
: QObject (parent)
, Imp_ (new QFile)
{
}

File::File (const File& file)
: QObject (file.parent ())
, Imp_ (new QFile (file.fileName ()))
{
}

File::~File ()
{
}

bool File::atEnd () const
{
	return Imp_->atEnd ();
}

qint64 File::bytesAvailable () const
{
	return Imp_->bytesAvailable ();
}

qint64 File::bytesToWrite () const
{
	return Imp_->bytesToWrite ();
}

bool File::canReadLine () const
{
	return Imp_->canReadLine ();
}

void File::close ()
{
	Imp_->close ();
}

bool File::copy (const QString& newName)
{
	return Imp_->copy (newName);
}

QFile::FileError File::error () const
{
	return Imp_->error ();
}

QString File::errorString () const
{
	return Imp_->errorString ();
}

bool File::exists () const
{
	return Imp_->exists ();
}

QString File::fileName () const
{
	return Imp_->fileName ();
}

bool File::flush ()
{
	return Imp_->flush ();
}

int File::handle () const
{
	return Imp_->handle ();
}

bool File::isOpen () const
{
	return Imp_->isOpen ();
}

bool File::isReadable () const
{
	return Imp_->isReadable ();
}

bool File::isSequential () const
{
	return Imp_->isSequential ();
}

bool File::isTextModeEnabled () const
{
	return Imp_->isTextModeEnabled ();
}

bool File::isWritable () const
{
	return Imp_->isWritable ();
}

bool File::link (const QString& link)
{
	return Imp_->link (link);
}

bool File::open (QIODevice::OpenMode mode)
{
	return Imp_->open (mode);
}

QIODevice::OpenMode File::openMode () const
{
	return Imp_->openMode ();
}

ByteArray File::peek (qint64 size)
{
	return Imp_->peek (size);
}

QFile::Permissions File::permissions () const
{
	return Imp_->permissions ();
}

qint64 File::pos () const
{
	return Imp_->pos ();
}

bool File::putChar (char c)
{
	return Imp_->putChar (c);
}

ByteArray File::read (qint64 size)
{
	return Imp_->read (size);
}

ByteArray File::readAll ()
{
	return Imp_->readAll ();
}

ByteArray File::readLine (qint64 size)
{
	return Imp_->readLine (size);
}

bool File::remove ()
{
	return Imp_->remove ();
}

bool File::rename (const QString& name)
{
	return Imp_->rename (name);
}

bool File::reset ()
{
	return Imp_->reset ();
}

bool File::resize (qint64 size)
{
	return Imp_->resize (size);
}

bool File::seek (qint64 pos)
{
	return Imp_->seek (pos);
}

void File::setFileName (const QString& name)
{
	Imp_->setFileName (name);
}

bool File::setPermissions (QFile::Permissions perms)
{
	return Imp_->setPermissions (perms);
}

void File::setTextModeEnabled (bool t)
{
	Imp_->setTextModeEnabled (t);
}

qint64 File::size () const
{
	return Imp_->size ();
}

QString File::symLinkTarget () const
{
	return Imp_->symLinkTarget ();
}

void File::unsetError ()
{
	Imp_->unsetError ();
}

bool File::waitForBytesWritten (int m)
{
	return Imp_->waitForBytesWritten (m);
}

bool File::waitForReadyRead (int m)
{
	return Imp_->waitForReadyRead (m);
}

qint64 File::write (const ByteArray& data)
{
	return Imp_->write (data);
}

QScriptValue toScriptValue (QScriptEngine *e, const QIODevice::OpenMode& om)
{
	QScriptValue obj = e->newObject ();
	obj.setProperty ("OpenMode", QScriptValue (e, om));
	return obj;
}

void fromScriptValue (const QScriptValue& sv, QIODevice::OpenMode& om)
{
	om = static_cast<QIODevice::OpenMode> (sv.property ("OpenMode").toInt32 ());
}

