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

#include "structuresops.h"

QDataStream& operator<< (QDataStream& out, const LeechCraft::DownloadEntity& e)
{
	quint16 version = 2;
	out << version
		<< e.Entity_
		<< e.Location_
		<< e.Mime_
		<< (int) e.Parameters_
		<< e.Additional_;
	return out;
}


QDataStream& operator>> (QDataStream& in, LeechCraft::DownloadEntity& e)
{
	quint16 version;
	in >> version;
	if (version == 2)
	{
		int parameters;
		in >> e.Entity_
			>> e.Location_
			>> e.Mime_
			>> parameters
			>> e.Additional_;

		if (parameters & LeechCraft::NoAutostart)
			e.Parameters_ |= LeechCraft::NoAutostart;
		if (parameters & LeechCraft::DoNotSaveInHistory)
			e.Parameters_ |= LeechCraft::DoNotSaveInHistory;
		if (parameters & LeechCraft::IsDownloaded)
			e.Parameters_ |= LeechCraft::IsDownloaded;
		if (parameters & LeechCraft::FromUserInitiated)
			e.Parameters_ |= LeechCraft::FromUserInitiated;
		if (parameters & LeechCraft::DoNotNotifyUser)
			e.Parameters_ |= LeechCraft::DoNotNotifyUser;
		if (parameters & LeechCraft::Internal)
			e.Parameters_ |= LeechCraft::Internal;
		if (parameters & LeechCraft::NotPersistent)
			e.Parameters_ |= LeechCraft::NotPersistent;
		if (parameters & LeechCraft::DoNotAnnounceEntity)
			e.Parameters_ |= LeechCraft::DoNotAnnounceEntity;
	}
	else if (version == 1)
	{
		QByteArray buf;
		int parameters;
		in >> buf
			>> e.Location_
			>> e.Mime_
			>> parameters
			>> e.Additional_;

		e.Entity_ = buf;

		if (parameters & LeechCraft::NoAutostart)
			e.Parameters_ |= LeechCraft::NoAutostart;
		if (parameters & LeechCraft::DoNotSaveInHistory)
			e.Parameters_ |= LeechCraft::DoNotSaveInHistory;
		if (parameters & LeechCraft::IsDownloaded)
			e.Parameters_ |= LeechCraft::IsDownloaded;
		if (parameters & LeechCraft::FromUserInitiated)
			e.Parameters_ |= LeechCraft::FromUserInitiated;
		if (parameters & LeechCraft::DoNotNotifyUser)
			e.Parameters_ |= LeechCraft::DoNotNotifyUser;
		if (parameters & LeechCraft::Internal)
			e.Parameters_ |= LeechCraft::Internal;
		if (parameters & LeechCraft::NotPersistent)
			e.Parameters_ |= LeechCraft::NotPersistent;
		if (parameters & LeechCraft::DoNotAnnounceEntity)
			e.Parameters_ |= LeechCraft::DoNotAnnounceEntity;
	}
	else
	{
		qWarning () << Q_FUNC_INFO
			<< "unknown version"
			<< "version";
	}
	return in;
}

namespace LeechCraft
{
	uint qHash (const LeechCraft::Notification& n)
	{
		return qHash (QString::number (n.Priority_) +
				(n.UntilUserSees_ ? 'a' : 'b') +
				n.Header_ +
				n.Text_);
	}
};

bool operator== (const LeechCraft::Notification& n1, const LeechCraft::Notification& n2)
{
	return n1.Priority_ == n2.Priority_ &&
		n1.UntilUserSees_ == n2.UntilUserSees_ &&
		n1.Header_ == n2.Header_ &&
		n1.Text_ == n2.Text_;
}

