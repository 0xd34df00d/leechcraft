/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "structuresops.h"
#include <QtDebug>

QDataStream& operator<< (QDataStream& out, const LC::Entity& e)
{
	quint16 version = 2;
	out << version
		<< e.Entity_
		<< e.Location_
		<< e.Mime_
		<< static_cast<quint32> (e.Parameters_)
		<< e.Additional_;
	return out;
}

QDataStream& operator>> (QDataStream& in, LC::Entity& e)
{
	quint16 version = 0;
	in >> version;
	if (version == 2)
	{
		quint32 parameters = 0;
		in >> e.Entity_
			>> e.Location_
			>> e.Mime_
			>> parameters
			>> e.Additional_;

		if (parameters & LC::NoAutostart)
			e.Parameters_ |= LC::NoAutostart;
		if (parameters & LC::DoNotSaveInHistory)
			e.Parameters_ |= LC::DoNotSaveInHistory;
		if (parameters & LC::IsDownloaded)
			e.Parameters_ |= LC::IsDownloaded;
		if (parameters & LC::FromUserInitiated)
			e.Parameters_ |= LC::FromUserInitiated;
		if (parameters & LC::DoNotNotifyUser)
			e.Parameters_ |= LC::DoNotNotifyUser;
		if (parameters & LC::Internal)
			e.Parameters_ |= LC::Internal;
		if (parameters & LC::NotPersistent)
			e.Parameters_ |= LC::NotPersistent;
		if (parameters & LC::DoNotAnnounceEntity)
			e.Parameters_ |= LC::DoNotAnnounceEntity;
		if (parameters & LC::OnlyHandle)
			e.Parameters_ |= LC::OnlyHandle;
		if (parameters & LC::OnlyDownload)
			e.Parameters_ |= LC::OnlyDownload;
		if (parameters & LC::AutoAccept)
			e.Parameters_ |= LC::AutoAccept;
		if (parameters & LC::FromCommandLine)
			e.Parameters_ |= LC::FromCommandLine;
	}
	else if (version == 1)
	{
		QByteArray buf;
		quint32 parameters = 0;
		in >> buf
			>> e.Location_
			>> e.Mime_
			>> parameters
			>> e.Additional_;

		e.Entity_ = buf;

		if (parameters & LC::NoAutostart)
			e.Parameters_ |= LC::NoAutostart;
		if (parameters & LC::DoNotSaveInHistory)
			e.Parameters_ |= LC::DoNotSaveInHistory;
		if (parameters & LC::IsDownloaded)
			e.Parameters_ |= LC::IsDownloaded;
		if (parameters & LC::FromUserInitiated)
			e.Parameters_ |= LC::FromUserInitiated;
		if (parameters & LC::DoNotNotifyUser)
			e.Parameters_ |= LC::DoNotNotifyUser;
		if (parameters & LC::Internal)
			e.Parameters_ |= LC::Internal;
		if (parameters & LC::NotPersistent)
			e.Parameters_ |= LC::NotPersistent;
		if (parameters & LC::DoNotAnnounceEntity)
			e.Parameters_ |= LC::DoNotAnnounceEntity;
		if (parameters & LC::OnlyHandle)
			e.Parameters_ |= LC::OnlyHandle;
		if (parameters & LC::OnlyDownload)
			e.Parameters_ |= LC::OnlyDownload;
		if (parameters & LC::AutoAccept)
			e.Parameters_ |= LC::AutoAccept;
		if (parameters & LC::FromCommandLine)
			e.Parameters_ |= LC::FromCommandLine;
	}
	else
	{
		qWarning () << Q_FUNC_INFO
			<< "unknown version"
			<< "version";
	}
	return in;
}

namespace LC
{
	bool operator< (const LC::Entity& e1, const LC::Entity& e2)
	{
		return e1.Mime_ < e2.Mime_ &&
			e1.Location_ < e2.Location_ &&
			e1.Parameters_ < e2.Parameters_;
	}

	bool operator== (const LC::Entity& e1, const LC::Entity& e2)
	{
		return e1.Mime_ == e2.Mime_ &&
			e1.Entity_ == e2.Entity_ &&
			e1.Location_ == e2.Location_ &&
			e1.Parameters_ == e2.Parameters_ &&
			e1.Additional_ == e2.Additional_;
	}
}
