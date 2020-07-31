/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QVariant>
#include <QDomElement>
#include <QScriptEngine>
#include "settings.h"

namespace LC
{
	class Scripter
	{
		std::unique_ptr<QScriptEngine> Engine_;
		std::unique_ptr<Settings> Settings_;
		QDomElement Container_;
	public:
		Scripter (const QDomElement&);

		QStringList GetOptions ();
		QString HumanReadableOption (const QString&);
	private:
		QString GetScript (const QDomElement&) const;
		void FeedRequiredClasses () const;
		void Reset ();
	};
};
