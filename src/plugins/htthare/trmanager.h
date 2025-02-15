/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <unordered_map>
#include <QMutex>
#include <QTranslator>

namespace LC::HttHare
{
	class TrManager : public QObject
	{
		QMutex MapLock_;

		struct Translator
		{
			std::unique_ptr<QTranslator> Translator_;
			QMutex TrMutex_;
		};
		std::unordered_map<QString, std::optional<Translator>> Translators_;
	public:
		using QObject::QObject;

		QString Translate (const QStringList& locales, const char *context, const char *src);
	private:
		std::optional<Translator>& GetTranslator (QString locale);
	};
}
