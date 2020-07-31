/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unclosemanager.h"
#include <functional>
#include <QMenu>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include "tabspropsmanager.h"
#include "util.h"

namespace LC
{
namespace TabSessManager
{
	UncloseManager::UncloseManager (const ICoreProxy_ptr& proxy, TabsPropsManager *tpm, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	, TabsPropsMgr_ { tpm }
	, UncloseMenu_ { new QMenu { tr ("Unclose tabs") } }
	{
	}

	QAction* UncloseManager::GetMenuAction () const
	{
		return UncloseMenu_->menuAction ();
	}

	void UncloseManager::HandleRemoveTab (QWidget *widget)
	{
		const auto tab = qobject_cast<ITabWidget*> (widget);
		if (!tab)
			return;

		if (const auto recTab = qobject_cast<IRecoverableTab*> (widget))
			HandleRemoveRecoverableTab (widget, recTab);
		else if (IsGoodSingleTC (tab->GetTabClassInfo ()))
			HandleRemoveSingleTab (widget, tab);
	}

	struct UncloseManager::RemoveTabParams
	{
		QByteArray RecoverData_;
		QString TabName_;
		QIcon TabIcon_;
		QWidget *Widget_;

		std::function<void (QObject*, TabRecoverInfo)> Uncloser_;
	};

	void UncloseManager::GenericRemoveTab (const RemoveTabParams& params)
	{
		TabRecoverInfo info
		{
			params.RecoverData_,
			GetSessionProps (params.Widget_)
		};

		const auto tab = qobject_cast<ITabWidget*> (params.Widget_);

		const auto rootWM = Proxy_->GetRootWindowsManager ();
		const auto winIdx = rootWM->GetWindowForTab (tab);
		const auto tabIdx = rootWM->GetTabWidget (winIdx)->IndexOf (params.Widget_);
		info.DynProperties_.append ({ "TabSessManager/Position", tabIdx });

		for (const auto& action : UncloseMenu_->actions ())
			if (action->property ("RecData") == params.RecoverData_)
			{
				UncloseMenu_->removeAction (action);
				action->deleteLater ();
				break;
			}

		const auto& fm = UncloseMenu_->fontMetrics ();
		const auto& elided = fm.elidedText (params.TabName_, Qt::ElideMiddle, 300);
		const auto action = new QAction { params.TabIcon_, elided, this };
		action->setProperty ("RecData", params.RecoverData_);

		const auto plugin = tab->ParentMultiTabs ();
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[uncloser = params.Uncloser_, info, plugin, action, winIdx, this]
			{
				action->deleteLater ();

				if (UncloseMenu_->defaultAction () == action)
					if (const auto nextAct = UncloseMenu_->actions ().value (1))
					{
						UncloseMenu_->setDefaultAction (nextAct);
						nextAct->setShortcut (QString ("Ctrl+Shift+T"));
					}
				UncloseMenu_->removeAction (action);

				const auto propsGuard = TabsPropsMgr_->AppendProps (info.DynProperties_);
				const auto winGuard = TabsPropsMgr_->AppendWindow (winIdx);
				uncloser (plugin, info);
			},
			action,
			SIGNAL (triggered ()),
			action
		};

		if (UncloseMenu_->defaultAction ())
			UncloseMenu_->defaultAction ()->setShortcut (QKeySequence ());
		UncloseMenu_->insertAction (UncloseMenu_->actions ().value (0), action);
		UncloseMenu_->setDefaultAction (action);
		action->setShortcut (QString ("Ctrl+Shift+T"));
	}

	void UncloseManager::HandleRemoveRecoverableTab (QWidget *widget, IRecoverableTab *recTab)
	{
		const auto& recoverData = recTab->GetTabRecoverData ();
		if (recoverData.isEmpty ())
			return;

		GenericRemoveTab ({
				recoverData,
				recTab->GetTabRecoverName (),
				recTab->GetTabRecoverIcon (),
				widget,
				[] (QObject *plugin, const TabRecoverInfo& info)
				{
					qobject_cast<IHaveRecoverableTabs*> (plugin)->RecoverTabs ({ info });
				}
			});
	}

	void UncloseManager::HandleRemoveSingleTab (QWidget *widget, ITabWidget *tab)
	{
		const auto& tc = tab->GetTabClassInfo ();
		GenericRemoveTab ({
				tc.TabClass_,
				tc.VisibleName_,
				tc.Icon_,
				widget,
				[] (QObject *plugin, const TabRecoverInfo& info)
				{
					qobject_cast<IHaveTabs*> (plugin)->TabOpenRequested (info.Data_);
				}
			});
	}
}
}
