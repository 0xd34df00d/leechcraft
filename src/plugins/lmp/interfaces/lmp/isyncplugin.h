/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QFile>
#include <QModelIndex>
#include <util/sll/either.h>
#include <util/threads/coro/taskfwd.h>
#include <util/lmp/mediainfo.h>

class QAbstractItemModel;

namespace LC::LMP
{
	enum class SyncConfLevel
	{
		None,
		Medium,
		High
	};

	class ISyncPluginConfig
	{
	public:
		virtual ~ISyncPluginConfig () = default;
	};

	using ISyncPluginConfig_cptr = std::shared_ptr<const ISyncPluginConfig>;

	class ISyncPluginConfigWidget
	{
	public:
		virtual ~ISyncPluginConfigWidget () = default;

		virtual QWidget* GetQWidget () = 0;

		virtual ISyncPluginConfig_cptr GetConfig () const = 0;
	};

	using ISyncPluginConfigWidget_ptr = std::unique_ptr<ISyncPluginConfigWidget>;

	class ISyncPlugin
	{
	public:
		virtual ~ISyncPlugin () = default;

		virtual QObject* GetQObject () = 0;

		virtual QString GetSyncSystemName () const = 0;

		struct Target
		{
			QString VisibleName_;
			QVariant Payload_;

			bool operator== (const Target& other) const
			{
				return Payload_ == other.Payload_;
			}
		};

		virtual QAbstractItemModel& GetSyncTargetsModel () const = 0;

		virtual void RefreshSyncTargets () = 0;

		/** @brief Returns the configuration widget for this sync method.
		 *
		 * If the sync method has no configuration, a nullptr can be returned.
		 */
		virtual ISyncPluginConfigWidget_ptr MakeConfigWidget () = 0;

		struct UploadSuccess {};
		struct UploadFailure
		{
			QFile::FileError Error_;
			QString ErrorStr_;
		};

		using UploadResult = Util::Either<UploadFailure, UploadSuccess>;

		struct UploadJob
		{
			QString LocalPath_;
			QString OriginalLocalPath_;

			MediaInfo MediaInfo_;

			QModelIndex Target_;

			ISyncPluginConfig_cptr Config_;
		};

		virtual Util::ContextTask<UploadResult> Upload (UploadJob uploadJob) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::LMP::ISyncPlugin, "org.LeechCraft.LMP.ISyncPlugin/1.0")
