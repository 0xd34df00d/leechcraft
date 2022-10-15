/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QTreeView;
class QLineEdit;

namespace LC
{
namespace Azoth
{
	class KeyboardRosterFixer : public QObject
	{
		QLineEdit * const Edit_;
		QTreeView * const View_;
		bool IsSearching_ = false;
		bool InterceptEnter_ = true;
	public:
		KeyboardRosterFixer (QLineEdit*, QTreeView*, QObject* = 0);

		void SetInterceptEnter (bool);
	protected:
		bool eventFilter (QObject*, QEvent*);
	};
}
}
