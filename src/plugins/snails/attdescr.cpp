/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "attdescr.h"
#include <QDataStream>
#include <QtDebug>
#include "vmimeconversions.h"

namespace LC
{
namespace Snails
{
	AttDescr::AttDescr (const QString& name, const QString& descr,
			const QByteArray& type, const QByteArray& subtype,
			qlonglong size)
	: Name_ (name)
	, Descr_ (descr)
	, Size_ (size)
	, Type_ (type)
	, SubType_ (subtype)
	{
	}

	QString AttDescr::GetName () const
	{
		return Name_;
	}

	QString AttDescr::GetDescr () const
	{
		return Descr_;
	}

	qlonglong AttDescr::GetSize () const
	{
		return Size_;
	}

	QByteArray AttDescr::GetType () const
	{
		return Type_;
	}

	QByteArray AttDescr::GetSubType () const
	{
		return SubType_;
	}

	QByteArray AttDescr::Serialize () const
	{
		QByteArray res;

		QDataStream str (&res, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
			<< Name_
			<< Descr_
			<< Size_
			<< Type_
			<< SubType_;

		return res;
	}

	void AttDescr::Deserialize (const QByteArray& serialized)
	{
		QDataStream str (serialized);

		quint8 version = 0;
		str >> version;
		if (version != 1)
			throw std::runtime_error (qPrintable ("Failed to deserialize AttDescr: unknown version " + QString::number (version)));

		str >> Name_
			>> Descr_
			>> Size_
			>> Type_
			>> SubType_;
	}

	void AttDescr::Dump () const
	{
		qDebug () << Q_FUNC_INFO
				<< Name_
				<< Type_ + '/' + SubType_
				<< Size_
				<< Descr_;
	}

	QDataStream& operator<< (QDataStream& str, const AttDescr& descr)
	{
		str << descr.Serialize ();
		return str;
	}

	QDataStream& operator>> (QDataStream& str, AttDescr& descr)
	{
		QByteArray data;
		str >> data;
		descr.Deserialize (data);
		return str;
	}
}
}
