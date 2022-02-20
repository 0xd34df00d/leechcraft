/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_searchwidget.h"

namespace LC
{
namespace Azoth
{
	class IHaveSearch;
	class ISearchSession;
	class AvatarsManager;

	class SearchWidget : public QWidget
					   , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentMultiTabs_;

		AvatarsManager * const AvatarsManager_;

		Ui::SearchWidget Ui_;
		std::shared_ptr<ISearchSession> CurrentSess_;
	public:
		static void SetParentMultiTabs (QObject*);

		explicit SearchWidget (AvatarsManager*, QWidget* = nullptr);

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;
	private:
		IHaveSearch* GetCurrentSearch () const;
	private slots:
		void search ();
		void on_AccountBox__activated (int);
	signals:
		void removeTab () override;
	};
}
}
