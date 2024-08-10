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
#include <QNetworkCookie>
#include <QtDebug>
#include <util/network/customcookiejar.h>
#include <util/sll/qtutil.h>
#include "xmlsettingsmanager.h"

namespace LC
{
	namespace
	{
		bool IsSaveEnabled ()
		{
			return !XmlSettingsManager::Instance ()->property ("DeleteCookiesOnExit").toBool ();
		}

		std::unique_ptr<QFile> GetCookiesFile (QIODevice::OpenMode mode)
		{
			if (!IsSaveEnabled ())
				return {};

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

		connect (&Jar_,
				&Util::CustomCookieJar::cookiesAdded,
				this,
				[this] (const QList<QNetworkCookie>& cookies)
				{
					AppendQueue_ += cookies;
					ScheduleSave ();
				});
		connect (&Jar_,
				&Util::CustomCookieJar::cookiesRemoved,
				this,
				[this]
				{
					HasRemovedCookies_ = true;
					ScheduleSave ();
				});
	}

	CookieSaver::~CookieSaver ()
	{
		FullSave ();
	}

	void CookieSaver::ScheduleSave ()
	{
		if (SaveScheduled_)
			return;

		SaveScheduled_ = true;

		using namespace std::chrono_literals;
		QTimer::singleShot (10s,
				this,
				&CookieSaver::Save);
	}

	void CookieSaver::Save ()
	{
		SaveScheduled_ = false;

		if (HasRemovedCookies_)
			FullSave ();
		else if (const auto file = GetCookiesFile (QIODevice::WriteOnly | QIODevice::Append))
			file->write (Util::CustomCookieJar::Save (AppendQueue_));

		HasRemovedCookies_ = false;
		AppendQueue_.clear ();
	}

	void CookieSaver::FullSave ()
	{
		if (const auto file = GetCookiesFile (QIODevice::WriteOnly | QIODevice::Truncate))
			file->write (Jar_.Save ());
	}
}
