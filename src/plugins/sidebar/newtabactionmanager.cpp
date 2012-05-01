/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "newtabactionmanager.h"
#include <QAction>
#include <QVariant>
#include "sbwidget.h"
#include "showconfigdialog.h"

namespace LeechCraft
{
namespace Sidebar
{
	NewTabActionManager::NewTabActionManager (SBWidget *w, QObject *parent)
	: QObject (parent)
	, Bar_ (w)
	, CfgDialog_ (new ShowConfigDialog ("NewTabActions"))
	{
		connect (CfgDialog_.get (),
				SIGNAL (showActions (QList<QAction*>)),
				this,
				SLOT (handleShowActions (QList<QAction*>)));
		connect (CfgDialog_.get (),
				SIGNAL (hideActions (QList<QAction*>)),
				this,
				SLOT (handleHideActions (QList<QAction*>)));

		QAction *cfgAction = new QAction (tr ("Configure new tab openers..."), this);
		connect (cfgAction,
				SIGNAL (triggered ()),
				CfgDialog_.get (),
				SLOT (show ()));
		w->addAction (cfgAction);
	}

	void NewTabActionManager::AddTabClassOpener (const TabClassInfo& tc, QObject *obj)
	{
		if (!(tc.Features_ & TabFeature::TFOpenableByRequest) ||
				(tc.Features_ & TabFeature::TFSingle))
			return;
		if (tc.Icon_.isNull ())
			return;

		QAction *act = new QAction (tc.Icon_,
				tc.VisibleName_, this);
		act->setToolTip (QString ("%1 (%2)")
				.arg (tc.VisibleName_)
				.arg (tc.Description_));
		act->setProperty ("Sidebar/Object",
				QVariant::fromValue<QObject*> (obj));
		act->setProperty ("Sidebar/TabClass", tc.TabClass_);
		connect (act,
				SIGNAL (triggered (bool)),
				this,
				SLOT (openNewTab ()));

		const bool showByDefault = tc.Features_ & TFSuggestOpening;
		if (CfgDialog_->CheckAction (tc.TabClass_, act, showByDefault))
			Bar_->AddTabOpenAction (act);
	}

	void NewTabActionManager::openNewTab ()
	{
		QObject *pluginObj = sender ()->
				property ("Sidebar/Object").value<QObject*> ();
		const QByteArray& tc = sender ()->
				property ("Sidebar/TabClass").toByteArray ();

		IHaveTabs *iht = qobject_cast<IHaveTabs*> (pluginObj);
		iht->TabOpenRequested (tc);
	}

	void NewTabActionManager::handleShowActions (const QList<QAction*>& acts)
	{
		Q_FOREACH (QAction *act, acts)
			Bar_->AddTabOpenAction (act);
	}

	void NewTabActionManager::handleHideActions (const QList<QAction*>& acts)
	{
		Q_FOREACH (QAction *act, acts)
			Bar_->RemoveTabOpenAction (act);
	}
}
}
