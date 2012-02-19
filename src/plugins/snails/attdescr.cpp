/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "attdescr.h"
#include <QtDebug>
#include "vmimeconversions.h"

namespace LeechCraft
{
namespace Snails
{
	AttDescr::AttDescr ()
	: Size_ (0)
	{
	}

	AttDescr::AttDescr (vmime::ref<const vmime::attachment> att)
	: Name_ (StringizeCT (att->getName ()))
	, Descr_ (StringizeCT (att->getDescription ()))
	, Size_ (att->getData ()->getLength ())
	, Type_ (att->getType ().getType ().c_str ())
	, SubType_ (att->getType ().getSubType ().c_str ())
	{
	}

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
