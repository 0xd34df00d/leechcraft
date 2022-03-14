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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/lmp/ilmpplugin.h>
#include "ui_checktab.h"

class QSortFilterProxyModel;

class QQuickWidget;

namespace LC::LMP::BrainSlugz
{
	class CheckModel;
	class Checker;

	class CheckTab : public QWidget
				   , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::CheckTab Ui_;
		QQuickWidget * const CheckView_;

		const ICoreProxy_ptr CoreProxy_;
		const TabClassInfo TC_;
		QObject * const Plugin_;

		QToolBar * const Toolbar_;

		CheckModel * const Model_;
		QAbstractItemModel * const CheckedModel_;

		bool IsRunning_ = false;
	public:
		CheckTab (const ILMPProxy_ptr&, const ICoreProxy_ptr&,
				const TabClassInfo& tc, QObject *plugin);

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;
	private:
		void SetupToolbar ();
		void Start ();
	signals:
		void removeTab () override;

		void runningStateChanged (bool);
		void checkStarted (Checker*);
	};
}
