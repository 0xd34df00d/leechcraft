/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncmanager.h"
#include <QStringList>
#include <QtDebug>
#include <QFileInfo>
#include <util/util.h>
#include <util/lmp/util.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sll/functor.h>
#include "copymanager.h"
#include "../core.h"
#include "../localfileresolver.h"

namespace LC
{
namespace LMP
{
	struct SyncManager::CopyJob
	{
		QObject* GetQObject () const
		{
			return Syncer_->GetQObject ();
		}

		void Upload () const
		{
			Syncer_->Upload (From_, OrigPath_, MountPoint_, Filename_);
		}

		QString From_;
		bool RemoveOnFinish_;

		ISyncPlugin *Syncer_;
		QString OrigPath_;
		QString MountPoint_;
		QString Filename_;
	};

	void SyncManager::AddFiles (ISyncPlugin *syncer, const QString& mount,
			const QStringList& files, const TranscodingParams& params)
	{
		for (const auto& file : files)
			Source2Params_ [file] = { syncer, mount };

		SyncManagerBase::AddFiles (files, params);
	}

	void SyncManager::CreateSyncer (const QString& mount)
	{
		auto mgr = new CopyManager<CopyJob> (this);
		connect (mgr,
				SIGNAL (startedCopying (QString)),
				this,
				SLOT (handleStartedCopying (QString)));
		connect (mgr,
				SIGNAL (finishedCopying ()),
				this,
				SLOT (handleFinishedCopying ()));
		connect (mgr,
				SIGNAL (copyProgress (qint64, qint64)),
				this,
				SLOT (handleCopyProgress (qint64, qint64)));
		connect (mgr,
				SIGNAL (errorCopying (QString, QString)),
				this,
				SLOT (handleErrorCopying (QString, QString)));
		Mount2Copiers_ [mount] = mgr;
	}

	namespace
	{
		Util::Either<ResolveError, QString> FixMask (const QString& mask, const QString& transcoded)
		{
			return Core::Instance ().GetLocalFileResolver ()->ResolveInfo (transcoded) *
					[&] (const MediaInfo& info)
					{
						auto result = PerformSubstitutions (mask, info, SubstitutionFlag::SFSafeFilesystem);
						const auto& ext = QFileInfo (transcoded).suffix ();
						if (!result.endsWith (ext))
							result += "." + ext;
						return result;
					};
		}
	}

	void SyncManager::handleFileTranscoded (const QString& from,
			const QString& transcoded, QString mask)
	{
		SyncManagerBase::HandleFileTranscoded (from, transcoded);

		const auto& syncTo = Source2Params_.take (from);
		if (syncTo.MountPath_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "dumb transcoded file detected"
					<< from
					<< transcoded;
			return;
		}

		emit uploadLog (tr ("File %1 successfully transcoded, adding to copy queue for the device %2...")
				.arg ("<em>" + QFileInfo (from).fileName () + "</em>")
				.arg ("<em>" + syncTo.MountPath_) + "</em>");

		Util::Visit (FixMask (mask, transcoded).AsVariant (),
				[&] (const QString& filename)
				{
					if (!Mount2Copiers_.contains (syncTo.MountPath_))
						CreateSyncer (syncTo.MountPath_);
					const CopyJob copyJob
					{
						transcoded,
						from != transcoded,
						syncTo.Syncer_,
						from,
						syncTo.MountPath_,
						filename
					};
					Mount2Copiers_ [syncTo.MountPath_]->Copy (copyJob);
				},
				[&] (const ResolveError& err)
				{
					const auto& errString = tr ("Unable to expand mask for file %1: %2.")
							.arg ("<em>" + QFileInfo { transcoded }.fileName () + "</em>")
							.arg (err.ReasonString_);
					emit uploadLog (errString);
					handleErrorCopying (transcoded, errString);
				});
	}
}
}
