/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>

namespace LC::Azoth
{
	class TabBase : public QWidget
				  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentMultiTabs_;
		static TabClassInfo S_TC_;
	public:
		using QWidget::QWidget;

		static void SetTabData (QObject*, const TabClassInfo&);

		TabClassInfo GetTabClassInfo () const final;
		QObject* ParentMultiTabs () final;
	signals:
		void removeTab () override;
	};
}
