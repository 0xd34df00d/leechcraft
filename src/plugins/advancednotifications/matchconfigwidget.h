/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/an/ianemitter.h>

namespace LC::AdvancedNotifications
{
	class IConfigWidget
	{
	protected:
		virtual ~IConfigWidget () = default;
	public:
		virtual QWidget& GetWidget () = 0;
		virtual AN::ValueMatcher GetConfiguredMatcher () const = 0;
	};

	using IConfigWidget_ptr = std::shared_ptr<IConfigWidget>;

	IConfigWidget_ptr CreateMatcherConfigWidget (const AN::FieldData&, const std::optional<AN::ValueMatcher>&);
	QString GetMatcherDescription (const AN::FieldData&, const AN::ValueMatcher&);
}
