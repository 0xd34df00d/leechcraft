/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiothememanager.h"
#include <util/sll/prelude.h>
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

	namespace
	{
		QStringList VariantsWithExtension (const QString& base)
		{
			static const QStringList exts
			{
				QStringLiteral (".ogg"),
				QStringLiteral (".wav"),
				QStringLiteral (".flac"),
				QStringLiteral (".mp3"),
			};
			return Util::Map (exts, [&base] (const QString& ext) { return base + ext; });
		}
	}

	QFileInfoList AudioThemeManager::GetFilesList (const QString& theme) const
	{
		static const auto filters = VariantsWithExtension (QStringLiteral ("*"));
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
		return Loader_->GetPath (VariantsWithExtension (base));
	}
}
}
