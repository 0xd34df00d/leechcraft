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
#include <QString>

class QAbstractItemModel;
class QFileInfo;

using QFileInfoList = QList<QFileInfo>;

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace AdvancedNotifications
{
	class AudioThemeManager : public QObject
	{
		std::shared_ptr<Util::ResourceLoader> Loader_;
	public:
		AudioThemeManager (QObject* = nullptr);

		QFileInfoList GetFilesList (const QString& theme) const;
		QAbstractItemModel* GetSettingsModel () const;

		QString GetAbsoluteFilePath (const QString&) const;
	};
}
}
