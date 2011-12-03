/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_ATTDESCR_H
#define PLUGINS_SNAILS_ATTDESCR_H
#include <QString>
#include <QMetaType>
#include <vmime/attachment.hpp>

namespace LeechCraft
{
namespace Snails
{
	class AttDescr
	{
		QString Name_;
		QString Descr_;
		qlonglong Size_;

		QByteArray Type_;
		QByteArray SubType_;
	public:
		AttDescr ();
		AttDescr (vmime::ref<const vmime::attachment>);
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

Q_DECLARE_METATYPE (LeechCraft::Snails::AttDescr);

#endif
