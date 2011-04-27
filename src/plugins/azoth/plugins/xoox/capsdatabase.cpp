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

#include "capsdatabase.h"
#include <QFile>
#include <QTimer>
#include <plugininterface/util.h>

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
	}
	
	bool CapsDatabase::Contains (const QByteArray& hash) const
	{
		return Ver2Features_.contains (hash);
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
		stream << static_cast<quint8> (1)
				<< Ver2Features_;
				
		SaveScheduled_ = false;
	}
	
	void CapsDatabase::ScheduleSave ()
	{
		if (SaveScheduled_)
			return;
		
		SaveScheduled_ = true;
		QTimer::singleShot (1000,
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
		if (ver == 1)
			stream >> Ver2Features_;
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown storage version"
					<< ver;
	}
}
}
}
