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
		if (parameters & LeechCraft::OnlyHandle)
			e.Parameters_ |= LeechCraft::OnlyHandle;
		if (parameters & LeechCraft::OnlyDownload)
			e.Parameters_ |= LeechCraft::OnlyDownload;
		if (parameters & LeechCraft::AutoAccept)
			e.Parameters_ |= LeechCraft::AutoAccept;
		if (parameters & LeechCraft::ShouldQuerySource)
			e.Parameters_ |= LeechCraft::ShouldQuerySource;
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
		if (parameters & LeechCraft::OnlyHandle)
			e.Parameters_ |= LeechCraft::OnlyHandle;
		if (parameters & LeechCraft::OnlyDownload)
			e.Parameters_ |= LeechCraft::OnlyDownload;
		if (parameters & LeechCraft::AutoAccept)
			e.Parameters_ |= LeechCraft::AutoAccept;
		if (parameters & LeechCraft::ShouldQuerySource)
			e.Parameters_ |= LeechCraft::ShouldQuerySource;
	}
	else
	{
		qWarning () << Q_FUNC_INFO
			<< "unknown version"
			<< "version";
	}
	return in;
}

bool operator< (const LeechCraft::DownloadEntity& e1, const LeechCraft::DownloadEntity& e2)
{
	return e1.Mime_ < e2.Mime_ &&
		e1.Location_ < e2.Location_ &&
		e1.Parameters_ < e2.Parameters_;
}

bool operator== (const LeechCraft::DownloadEntity& e1, const LeechCraft::DownloadEntity& e2)
{
	return e1.Mime_ == e2.Mime_ &&
		e1.Entity_ == e2.Entity_ &&
		e1.Location_ == e2.Location_ &&
		e1.Parameters_ == e2.Parameters_ &&
		e1.Additional_ == e2.Additional_;
}
