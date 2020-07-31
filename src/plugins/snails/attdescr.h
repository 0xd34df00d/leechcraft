/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QMetaType>
#include <vmime/attachment.hpp>

namespace LC
{
namespace Snails
{
	class AttDescr
	{
		QString Name_;
		QString Descr_;
		qlonglong Size_ = 0;

		QByteArray Type_;
		QByteArray SubType_;
	public:
		AttDescr () = default;
		AttDescr (const QString& name, const QString& descr,
				const QByteArray& type, const QByteArray& subtype,
				qlonglong size);

		QString GetName () const;
		QString GetDescr () const;
		qlonglong GetSize () const;

		QByteArray GetType () const;
		QByteArray GetSubType () const;

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);

		void Dump () const;
	};

	QDataStream& operator<< (QDataStream&, const AttDescr&);
	QDataStream& operator>> (QDataStream&, AttDescr&);
}
}

Q_DECLARE_METATYPE (LC::Snails::AttDescr)
