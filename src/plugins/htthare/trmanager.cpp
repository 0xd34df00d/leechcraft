/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trmanager.h"
#include <util/util.h>

namespace LC::HttHare
{
	QString TrManager::Translate (const QStringList& locales, const char* context, const char* src)
	{
		for (const auto& locale : locales)
			if (auto& translator = GetTranslator (locale))
			{
				const QMutexLocker locker { &translator->TrMutex_ };
				const auto& str = translator->Translator_->translate (context, src);
				if (!str.isEmpty ())
					return str;
			}
		return QString::fromUtf8 (src);
	}

	std::optional<TrManager::Translator>& TrManager::GetTranslator (QString locale)
	{
		if (locale.size () > 2)
			locale = locale.left (2);
		if (locale == "ru")
			locale = "ru_RU";

		const QMutexLocker lock { &MapLock_ };
		auto pos = Translators_.find (locale);
		if (pos == Translators_.end ())
		{
			if (auto translator = std::unique_ptr<QTranslator> (Util::LoadTranslator ("htthare", locale)))
				pos = Translators_.emplace (std::piecewise_construct,
						std::forward_as_tuple (locale),
						std::forward_as_tuple (std::in_place, std::move (translator))).first;
			else
				pos = Translators_.emplace (locale, std::nullopt).first;
		}
		return pos->second;
	}
}
