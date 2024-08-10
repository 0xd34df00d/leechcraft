/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cookiesaver.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QtDebug>
#include <util/network/customcookiejar.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include "entitymanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
	namespace
	{
		std::unique_ptr<QFile> GetCookiesFile (QIODevice::OpenMode mode)
		{
			auto dir = QDir::home ();
			const auto corePath = ".leechcraft/core"_qs;
			if (!dir.mkpath (corePath))
			{
				qCritical () << "unable to create path" << dir.filePath (corePath);
				return {};
			}

			dir.cd (corePath);
			const auto& path = dir.filePath ("cookies.txt"_qs);

			auto file = std::make_unique<QFile> (path);
			if (!file->open (mode))
			{
				qCritical () << "unable to open file" << path << file->errorString ();
				return {};
			}
			return file;
		}
	}

	CookieSaver::CookieSaver (Util::CustomCookieJar& jar, QObject *parent)
	: QObject { parent }
	, Jar_ { jar }
	{
		if (const auto file = GetCookiesFile (QIODevice::ReadOnly))
			Jar_.Load (file->readAll ());

		using namespace std::chrono_literals;

		const auto saveTimer = new QTimer { this };
		saveTimer->callOnTimeout (this, &CookieSaver::SaveCookies);
		saveTimer->start (10s);
	}

	CookieSaver::~CookieSaver ()
	{
		SaveCookies ();
	}

	void CookieSaver::SaveCookies ()
	{
		static int errorCount = 0;

		if (const auto file = GetCookiesFile (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			const bool saveEnabled = !XmlSettingsManager::Instance ()->property ("DeleteCookiesOnExit").toBool ();
			file->write (saveEnabled ? Jar_.Save () : QByteArray {});

			errorCount = 0;
		}
		else if (errorCount++ < 3)
		{
			EntityManager em { nullptr, nullptr };
			em.HandleEntity (Util::MakeNotification ("LeechCraft"_qs,
					tr ("Unable to save cookies."),
					Priority::Critical));
		}
	}
}
