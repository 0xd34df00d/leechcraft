/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Poshuku
{
	class IURLCompletionModel
	{
	public:
		virtual ~IURLCompletionModel () {}

		virtual void AddItem (const QString& title, const QString& url, size_t pos) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IURLCompletionModel,
		"org.Deviant.LeechCraft.Poshuku.IURLCompletionModel/1.0")
