/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QString>

template<typename>
class QList;

template<typename, typename>
class QHash;

namespace LC
{
enum class ContentType;

namespace Snails
{
	class Message;
	class Account;
	struct MessageInfo;
	struct MessageBodies;

	using PatternFunction_t = std::function<QString (const Account*, MessageInfo, MessageBodies, ContentType)>;

	struct TemplatePattern
	{
		QString PatternText_;

		PatternFunction_t Substitute_;
	};

	QList<TemplatePattern> GetKnownPatterns ();
	QHash<QString, PatternFunction_t> GetKnownPatternsHash ();
}
}
