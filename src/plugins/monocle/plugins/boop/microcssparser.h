/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QString>
#include <QVector>

namespace LC::Monocle::Boop::MicroCSS
{
	struct Rule
	{
		QString Property_;
		QString Value_;
	};

	QDebug operator<< (QDebug, const Rule&);

	struct Stylesheet
	{
		QHash<QString, QVector<Rule>> Selectors_;

		Stylesheet& operator+= (const Stylesheet&);
	};

	Stylesheet Parse (QStringView str, const std::function<bool (QString)>& selectorFilter);
}
