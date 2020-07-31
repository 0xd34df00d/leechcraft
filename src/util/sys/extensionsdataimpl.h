/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QHash>
#include <QString>

class QIcon;

namespace LC
{
namespace Util
{
	class ExtensionsDataImpl
	{
		struct Details;
		const std::shared_ptr<Details> Details_ {};
	public:
		ExtensionsDataImpl ();

		const QHash<QString, QString>& GetMimeDatabase () const;
		QIcon GetExtIcon (const QString& extension) const;
		QIcon GetMimeIcon (const QString& mime) const;
	};
}
}
