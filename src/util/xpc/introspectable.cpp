/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "introspectable.h"
#include <string>
#include <QtDebug>

namespace LC
{
namespace Util
{
	Introspectable& Introspectable::Instance ()
	{
		static Introspectable inst;
		return inst;
	}

	QVariantMap Introspectable::operator() (const QVariant& variant) const
	{
		if (!variant.isValid ())
			throw std::runtime_error { "Invalid variant." };

		const auto type = variant.userType ();

		if (type < static_cast<int> (QVariant::UserType))
			return { { "data", variant } };

		if (Intros_.contains (type))
			return Intros_ [type] (variant);

		qWarning () << Q_FUNC_INFO
				<< "unregistered type"
				<< type
				<< variant;
		throw std::runtime_error { "Unregistered type: " + std::to_string (type) };
	}
}
}
