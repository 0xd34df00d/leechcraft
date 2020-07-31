/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTreeView>

namespace LC::Fenet
{
	class CompParamsWidget : public QTreeView
	{
		Q_OBJECT
	public:
		explicit CompParamsWidget (QWidget* = nullptr);
	public slots:
		void accept ();
		void reject ();
	signals:
		void accepted ();
		void rejected ();
	};
}
