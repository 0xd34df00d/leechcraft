/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class Q_DECL_EXPORT IAdvancedPlainTextEditor
{
public:
	virtual ~IAdvancedPlainTextEditor () = default;

	virtual bool FindText (const QString&) = 0;

	virtual void DeleteSelection () = 0;
};

Q_DECLARE_INTERFACE (IAdvancedPlainTextEditor, "org.LeechCraft.IAdvancedPlainTextEditor/1.0")
