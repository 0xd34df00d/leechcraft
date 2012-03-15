/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "pintab.h"
#include <QIcon>
#include <QMenu>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/icoretabwidget.h>

namespace LeechCraft
{
namespace PinTab
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("pintab");

		MainTabWidget_ = proxy->GetTabWidget ();
		connect (MainTabWidget_->GetObject (),
				SIGNAL (tabInserted (int)),
				this,
				SLOT (checkPinState (int)));

		PinTab_ = new QAction (tr ("Pin tab"), this);
		connect (PinTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (pinTab ()));

		UnPinTab_ = new QAction (tr ("Unpin tab"), this);
		connect (UnPinTab_,
				SIGNAL (triggered ()),
				this,
				SLOT (unPinTab ()));

		CloseSide_ = MainTabWidget_->GetCloseButtonPosition ();

		Id_ = 0;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.PinTab";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "PinTab";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support tabs pinning.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/pintab/resources/images/pintab.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void Plugin::hookTabContextMenuFill (LeechCraft::IHookProxy_ptr proxy,
			QMenu *menu, int index)
	{
		QList<QAction*> actions = MainTabWidget_->GetPermanentActions ();
		QAction *firstAction = 0;
		if (!actions.isEmpty ())
			firstAction = actions.at (0);

		int realIndex = MainTabWidget_->TabData (index).toInt ();
		if (PinTabsIndex2TabData_.contains (realIndex))
		{
			menu->insertAction (firstAction, UnPinTab_);
			UnPinTab_->setProperty ("Leechcraft/PinTab/CurrentIndex", index);
		}
		else
		{
			menu->insertAction (firstAction, PinTab_);
			PinTab_->setProperty ("Leechcraft/PinTab/CurrentIndex", index);
		}
	}

	void Plugin::hookTabFinishedMoving (LeechCraft::IHookProxy_ptr proxy, int index)
	{
		int realIndex = MainTabWidget_->TabData (index).toInt ();
		int realNextIndex = MainTabWidget_->TabData (index + 1).toInt ();
		int realPrevIndex = MainTabWidget_->TabData (index - 1).toInt ();
		if (PinTabsIndex2TabData_.contains (realNextIndex) &&
				!PinTabsIndex2TabData_.contains (realIndex))
		{
			pinTab (index);
			MainTabWidget_->MoveTab (PinTabsIndex2TabData_.count () - 1, index);
		}
		else if (PinTabsIndex2TabData_.contains (realIndex) &&
				index &&
				!PinTabsIndex2TabData_.contains (realPrevIndex))
		{
			unPinTab (index);
			MainTabWidget_->MoveTab (PinTabsIndex2TabData_.count (), index);
		}
	}

	void Plugin::hookTabSetText (IHookProxy_ptr proxy, int index)
	{
		int realIndex = MainTabWidget_->TabData (index).toInt ();
		if (PinTabsIndex2TabData_.contains (realIndex))
			proxy->CancelDefault ();
	}

	void Plugin::pinTab (int index)
	{
		if (index == -1)
			index = sender ()->property ("Leechcraft/PinTab/CurrentIndex").toInt ();

		if (index < 0 ||
				index >= MainTabWidget_->WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		MainTabWidget_->Widget (index)->
				setProperty ("SessionData/org.LeechCraft.PinTab.PinState", true);
		++Id_;
		auto pair = qMakePair (MainTabWidget_->TabText (index),
				MainTabWidget_->TabButton (index, CloseSide_));
		MainTabWidget_->SetTabData (index, Id_);
		MainTabWidget_->SetTabText (index, "");
		MainTabWidget_->SetTabClosable (index, false);
		PinTabsIndex2TabData_ [Id_] = pair;

		MainTabWidget_->MoveTab (index, PinTabsIndex2TabData_.count () - 1);
	}

	void Plugin::unPinTab (int index)
	{
		if (index == -1)
			index = sender ()->property ("Leechcraft/PinTab/CurrentIndex").toInt ();

		if (index < 0 ||
				index >= MainTabWidget_->WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		int realIndex = MainTabWidget_->TabData (index).toInt ();
		MainTabWidget_->Widget (index)->
				setProperty ("SessionData/org.LeechCraft.PinTab.PinState", false);
		auto data = PinTabsIndex2TabData_.take (realIndex);

		MainTabWidget_->SetTabText (index, data.first);
		MainTabWidget_->SetTabClosable (index, true, data.second);

		MainTabWidget_->MoveTab (index, PinTabsIndex2TabData_.count ());
	}

	void Plugin::checkPinState (int index)
	{
		if (MainTabWidget_->WidgetCount () <= index)
			return;

		bool IsPinned = MainTabWidget_->Widget (index)->
				property ("SessionData/org.LeechCraft.PinTab.PinState").toBool ();

		if (IsPinned)
			pinTab (index);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pintab, LeechCraft::PinTab::Plugin);
