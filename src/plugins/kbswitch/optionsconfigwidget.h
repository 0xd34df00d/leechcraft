/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>

class QStandardItemModel;

namespace LC
{
namespace KBSwitch
{
	class OptionsConfigWidget : public QWidget
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		bool Modified_ = false;
	public:
		OptionsConfigWidget (QWidget* = 0);
	public slots:
		virtual void accept ();
		virtual void reject ();
	private slots:
		void markModified ();
	};
}
}
