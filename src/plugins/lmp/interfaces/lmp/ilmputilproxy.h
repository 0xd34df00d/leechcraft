/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <atomic>
#include <QtPlugin>
#include <QFileInfo>
#include <QMap>
#include <QVariant>

class QPixmap;

namespace LC
{
namespace LMP
{
	class ILMPUtilProxy
	{
	public:
		virtual ~ILMPUtilProxy () {}

		virtual QString FindAlbumArt (const QString& near, bool includeCollection = true) const = 0;

		virtual QList<QFileInfo> RecIterateInfo (const QString& dirPath,
				bool followSymlinks = false,
				std::atomic<bool> *stopGuard = nullptr) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ILMPUtilProxy, "org.LeechCraft.LMP.ILMPUtilProxy/1.0")
