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
#ifndef BYTEARRAY_H
#define BYTEARRAY_H
#include <memory>
#include <QObject>
#include <QByteArray>
#include <QScriptEngine>

class QString;

namespace LeechCraft
{
	class ByteArray : public QObject
	{
		Q_OBJECT

		std::auto_ptr<QByteArray> Imp_;
	public:
		ByteArray (QObject* = 0);
		ByteArray (const ByteArray&, QObject* = 0);
		ByteArray (const QByteArray&, QObject* = 0);
		virtual ~ByteArray ();

		Q_INVOKABLE ByteArray& operator= (const ByteArray&);
		Q_INVOKABLE bool operator!= (const QString&) const;
		Q_INVOKABLE ByteArray& operator+= (const ByteArray&);
		Q_INVOKABLE ByteArray& operator+= (const QString&);
		Q_INVOKABLE ByteArray& operator+= (char);
		Q_INVOKABLE bool operator< (const QString&) const;
		Q_INVOKABLE bool operator<= (const QString&) const;
		Q_INVOKABLE bool operator== (const QString&) const;
		Q_INVOKABLE bool operator> (const QString&) const;
		Q_INVOKABLE bool operator>= (const QString&) const;

		operator QByteArray () const;
	public slots:
		ByteArray& append (const ByteArray&);
		ByteArray& append (const QString&);
		ByteArray& append (char);
		char at (int) const;
		int capacity () const;
		void chop (int);
		void clear ();
		bool contains (const ByteArray&) const;
		bool contains (char) const;
		int count (const ByteArray&) const;
		int count (char) const;
		int count () const;
		bool endsWith (const ByteArray&) const;
		bool endsWith (char) const;
		ByteArray& fill (char, int = -1);
		int indexOf (const ByteArray&, int = 0) const;
		int indexOf (const QString&, int = 0) const;
		int indexOf (char, int = 0) const;
		ByteArray& insert (int, const ByteArray&);
		ByteArray& insert (int, const QString&);
		ByteArray& insert (int, char);
		bool isEmpty () const;
		bool isNull () const;
		int lastIndexOf (const ByteArray&, int = -1) const;
		int lastIndexOf (const QString&, int = -1) const;
		int lastIndexOf (char c, int = -1) const;
		ByteArray left (int) const;
		ByteArray leftJustified (int, char = ' ', bool = false) const;
		int length () const;
		ByteArray mid (int, int = -1) const;
		ByteArray& prepend (const ByteArray&);
		ByteArray& prepend (char c);
		void push_back (const ByteArray&);
		void push_back (char c);
		void push_front (const ByteArray&);
		void push_front (char c);
		ByteArray& remove (int, int);
		ByteArray& replace (int, int, const ByteArray&);
		ByteArray& replace (const ByteArray&, const ByteArray&);
		ByteArray& replace (const QString&, const ByteArray&);
		ByteArray& replace (char, const ByteArray&);
		ByteArray& replace (char, const QString&);
		ByteArray& replace (char, char);
		void reserve (int);
		void resize (int);
		ByteArray right (int) const;
		ByteArray rightJustified (int, char = ' ', bool = false) const;
		ByteArray& setNum (int, int = 10);
		ByteArray& setNum (uint, int = 10);
		ByteArray& setNum (short, int = 10);
		ByteArray& setNum (ushort, int = 10);
		ByteArray& setNum (qlonglong, int = 10);
		ByteArray& setNum (qulonglong, int = 10);
		ByteArray& setNum (double, char = 'g', int = 6);
		ByteArray& setNum (float, char = 'g', int = 6);
		ByteArray simplified () const;
		int size () const;
		QList<ByteArray> split (char) const;
		void squeeze ();
		bool startsWith (const ByteArray&) const;
		bool startsWith (char) const;
		ByteArray toBase64 () const;
		double toDouble () const;
		float toFloat () const;
		ByteArray toHex () const;
		int toInt (int = 10) const;
		long toLong (int = 10) const;
		qlonglong toLongLong (int = 10) const;
		ByteArray toLower () const;
		ByteArray toPercentEncoding (const ByteArray&,
				const ByteArray&, char = '%') const;
		short toShort (int = 10) const;
		uint toUInt (int = 10) const;
		ulong toULong (int = 10) const;
		qulonglong toULongLong (int = 10) const;
		ushort toUShort (int = 10) const;
		ByteArray toUpper () const;
		ByteArray trimmed () const;
		void truncate (int);
	};
};

Q_DECLARE_METATYPE (LeechCraft::ByteArray);
Q_SCRIPT_DECLARE_QMETAOBJECT (LeechCraft::ByteArray, QObject*);

#endif

