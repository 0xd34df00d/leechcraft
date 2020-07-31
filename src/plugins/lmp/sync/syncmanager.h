/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "syncmanagerbase.h"
#include "interfaces/lmp/isyncplugin.h"

namespace LC
{
namespace LMP
{
	class TranscodeManager;
	struct TranscodingParams;

	template<typename>
	class CopyManager;

	class SyncManager : public SyncManagerBase
	{
		Q_OBJECT

		struct CopyJob;
		QMap<QString, CopyManager<CopyJob>*> Mount2Copiers_;

		struct SyncTo
		{
			ISyncPlugin *Syncer_;
			QString MountPath_;
		};
		QMap<QString, SyncTo> Source2Params_;
	public:
		using SyncManagerBase::SyncManagerBase;

		void AddFiles (ISyncPlugin*, const QString& mount, const QStringList&, const TranscodingParams&);
	private:
		void CreateSyncer (const QString&);
	protected slots:
		void handleFileTranscoded (const QString& from, const QString&, QString);
	};
}
}
