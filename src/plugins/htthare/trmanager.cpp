/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trmanager.h"
#include <QThread>
#include <QTranslator>
#include <QTimer>
#include <util/util.h>

namespace LC
{
namespace HttHare
{
	TrManager::TrManager (QObject *parent)
	: QObject (parent)
	{
		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (purge ()));
		timer->start (60 * 60 * 1000);
	}

	QString TrManager::Translate (const QStringList& locales, const char* context, const char* src)
	{
		MapLock_.lock ();
		auto& map = Translators_ [QThread::currentThreadId ()];
		MapLock_.unlock ();

		for (auto locale : locales)
		{
			if (locale.size () > 2)
				locale = locale.left (2);

			if (locale == "ru")
				locale = "ru_RU";

			if (!map.contains (locale))
				map [locale] = Util::LoadTranslator ("htthare", locale);

			if (const auto transl = map [locale])
			{
				const auto& str = transl->translate (context, src);
				if (!str.isEmpty ())
					return str;
			}
		}

		return QString::fromUtf8 (src);
	}

	void TrManager::purge ()
	{
		MapLock_.lock ();
		for (const auto& threadMap : Translators_)
			for (auto tr : threadMap)
				tr->deleteLater ();
		Translators_.clear ();
		MapLock_.unlock ();
	}
}
}
