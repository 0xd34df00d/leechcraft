/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiothememanager.h"
#include <util/sys/resourceloader.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace AdvancedNotifications
{
	AudioThemeManager::AudioThemeManager (QObject *parent)
	: QObject { parent }
	, Loader_ { std::make_shared<Util::ResourceLoader> ("sounds/") }
	{
		Loader_->AddLocalPrefix ();
		Loader_->AddGlobalPrefix ();
	}

	QFileInfoList AudioThemeManager::GetFilesList (const QString& theme) const
	{
		static const QStringList filters
		{
			"*.ogg",
			"*.wav",
			"*.flac",
			"*.mp3"
		};

		return Loader_->List (theme, filters, QDir::Files | QDir::Readable);
	}

	QAbstractItemModel* AudioThemeManager::GetSettingsModel () const
	{
		return Loader_->GetSubElemModel ();
	}

	QString AudioThemeManager::GetAbsoluteFilePath (const QString& fname) const
	{
		if (fname.isEmpty ())
			return {};

		if (fname.contains ('/'))
			return fname;

		const auto& option = XmlSettingsManager::Instance ().property ("AudioTheme").toString ();
		const auto& base = option + '/' + fname;

		const QStringList pathVariants
		{
			base + ".ogg",
			base + ".wav",
			base + ".flac",
			base + ".mp3"
		};

		return Loader_->GetPath (pathVariants);
	}
}
}
