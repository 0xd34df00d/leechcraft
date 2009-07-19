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
