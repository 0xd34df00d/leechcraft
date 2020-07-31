/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

typedef struct _Itdb_iTunesDB Itdb_iTunesDB;
typedef struct _Itdb_Track Itdb_Track;

namespace LC
{
namespace LMP
{
struct UnmountableFileInfo;

namespace jOS
{
	class GpodDb : public QObject
	{
		Itdb_iTunesDB *DB_ = nullptr;
		const QString LocalPath_;
	public:
		GpodDb (const QString& localPath, QObject* = 0);
		~GpodDb ();

		enum class Result
		{
			Success,
			NotFound,
			OtherError
		};

		struct InitResult
		{
			Result Result_;
			QString Message_;
		};

		InitResult Reinitialize ();
		InitResult Load ();

		bool Save () const;

		Itdb_Track* AddTrack (const QString&, const QString&, const UnmountableFileInfo&);

		std::shared_ptr<void> GetSyncGuard () const;
	};
}
}
}
