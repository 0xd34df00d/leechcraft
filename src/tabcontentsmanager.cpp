#include "tabcontentsmanager.h"
#include <QToolBar>
#include "tabcontents.h"
#include "core.h"
#include "mainwindow.h"

namespace LeechCraft
{
	TabContentsManager::TabContentsManager ()
	{
	}

	TabContentsManager& TabContentsManager::Instance ()
	{
		static TabContentsManager tcm;
		return tcm;
	}

	void TabContentsManager::SetDefault (TabContents *tc)
	{
		Default_ = tc;
	}

	QList<TabContents*> TabContentsManager::GetTabs () const
	{
		QList<TabContents*> result;
		result.prepend (Default_);
		return result;
	}

	void TabContentsManager::AddNewTab ()
	{
		TabContents *tc = new TabContents ();
		Others_ << tc;
		emit addNewTab (tr ("Summary"), tc);
	}

	void TabContentsManager::RemoveTab (TabContents *tc)
	{
		emit removeTab (tc);
		Others_.removeAll (tc);
		delete tc;
	}

	void TabContentsManager::MadeCurrent (TabContents *tc)
	{
		Q_FOREACH (TabContents *tab, GetTabs ())
		{
			if (tab != tc)
			{
				tab->SmartDeselect ();
			}
		}

		if (tc)
		{
			QItemSelection sel = tc->GetUi ().PluginsTasksTree_->
				selectionModel ()->selection ();
			if (sel.size ())
			{
				QModelIndex ri = sel.at (0).topLeft ();
				QToolBar *controls = Core::Instance ()
							.GetControls (ri);
				QWidget *addiInfo = Core::Instance ()
							.GetAdditionalInfo (ri);

				if (controls)
				{
					controls->setFloatable (true);
					controls->setMovable (true);
					Core::Instance ().GetReallyMainWindow ()->addToolBar (controls);
					controls->show ();
				}
				if (addiInfo)
				{
					if (addiInfo->parent () != this)
						addiInfo->setParent (tc);
					tc->GetUi ().ControlsDockWidget_->setWidget (addiInfo);
					tc->GetUi ().ControlsDockWidget_->show ();
				}
			}
		}

		/* Hope this won't be needed with the SmartDeselect().
		else
		{
			QItemSelection sel = Ui_.PluginsTasksTree_->
				selectionModel ()->selection ();
			if (sel.size ())
			{
				QModelIndex ri = sel.at (0).topLeft ();
				QToolBar *controls = Core::Instance ()
							.GetControls (ri);

				if (controls)
					removeToolBar (controls);
			}

			CurrentToolBar_ = Core::Instance ().GetToolBar (index);
			if (CurrentToolBar_)
			{
				addToolBar (CurrentToolBar_);
				CurrentToolBar_->show ();
			}
		}
		*/
	}
};

