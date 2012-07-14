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

#include "capsdatabase.h"
#include <QFile>
#include <QTimer>
#include <util/util.h>

Q_DECLARE_METATYPE (QXmppDiscoveryIq::Identity);

QDataStream& operator<< (QDataStream& out, const QXmppDiscoveryIq::Identity& id)
{
	out << static_cast<quint8> (1)
		<< id.category ()
		<< id.language ()
		<< id.name ()
		<< id.type ();
	return out;
}

QDataStream& operator>> (QDataStream& in, QXmppDiscoveryIq::Identity& id)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
	{
		qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
		return in;
	}

	QString category, language, name, type;
	in >> category
		>> language
		>> name
		>> type;
	id.setCategory (category);
	id.setLanguage (language);
	id.setName (name);
	id.setType (type);

	return in;
}

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	CapsDatabase::CapsDatabase (QObject *parent)
	: QObject (parent)
	, SaveScheduled_ (false)
	{
		qRegisterMetaType<QXmppDiscoveryIq::Identity> ("QXmppDiscoveryIq::Identity");
		qRegisterMetaTypeStreamOperators<QXmppDiscoveryIq::Identity> ("QXmppDiscoveryIq::Identity");
		Load ();
	}

	bool CapsDatabase::Contains (const QByteArray& hash) const
	{
		return Ver2Features_.contains (hash) &&
				Ver2Identities_.contains (hash);
	}

	QStringList CapsDatabase::Get (const QByteArray& hash) const
	{
		return Ver2Features_ [hash];
	}

	void CapsDatabase::Set (const QByteArray& hash, const QStringList& features)
	{
		Ver2Features_ [hash] = features;
		ScheduleSave ();
	}

	QList<QXmppDiscoveryIq::Identity> CapsDatabase::GetIdentities (const QByteArray& hash) const
	{
		return Ver2Identities_ [hash];
	}

	void CapsDatabase::SetIdentities (const QByteArray& hash,
			const QList<QXmppDiscoveryIq::Identity>& ids)
	{
		Ver2Identities_ [hash] = ids;
		ScheduleSave ();
	}

	void CapsDatabase::save () const
	{
		QDir dir = Util::CreateIfNotExists ("azoth/xoox");
		QFile file (dir.filePath ("caps_s.db"));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for writing:"
					<< file.errorString ();
			return;
		}

		QDataStream stream (&file);
		stream << static_cast<quint8> (2)
				<< Ver2Features_
				<< Ver2Identities_;

		SaveScheduled_ = false;
	}

	void CapsDatabase::ScheduleSave ()
	{
		if (SaveScheduled_)
			return;

		SaveScheduled_ = true;
		QTimer::singleShot (10000,
				this,
				SLOT (save ()));
	}

	void CapsDatabase::Load ()
	{
		QDir dir = Util::CreateIfNotExists ("azoth/xoox");
		QFile file (dir.filePath ("caps_s.db"));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for reading:"
					<< file.errorString ();
			return;
		}

		QDataStream stream (&file);
		quint8 ver = 0;
		stream >> ver;
		if (ver < 1 || ver > 2)
			qWarning () << Q_FUNC_INFO
					<< "unknown storage version"
					<< ver;
		if (ver >= 1)
			stream >> Ver2Features_;
		if (ver >= 2)
			stream >> Ver2Identities_;
	}
}
}
}
