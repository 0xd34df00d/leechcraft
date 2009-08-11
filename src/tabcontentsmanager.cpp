#include "tabcontentsmanager.h"
#include <QToolBar>
#include "tabcontents.h"
#include "core.h"
#include "mainwindow.h"
#include "viewreemitter.h"
#include "interfaces/imultitabs.h"

namespace LeechCraft
{
	TabContentsManager::TabContentsManager ()
	: Reemitter_ (new ViewReemitter (this))
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
		Current_ = Default_;

		Connect (tc);
		Reemitter_->Connect (Default_);
	}

	QList<TabContents*> TabContentsManager::GetTabs () const
	{
		QList<TabContents*> result = Others_;
		result.prepend (Default_);
		return result;
	}

	void TabContentsManager::AddNewTab (const QString& query)
	{
		TabWidget *tw = Core::Instance ()
			.GetReallyMainWindow ()->GetTabWidget ();
		IMultiTabsWidget *imtw =
			qobject_cast<IMultiTabsWidget*> (tw->currentWidget ());
		if (imtw)
			imtw->NewTabRequested ();
		else
		{
			TabContents *tc = new TabContents ();
			Connect (tc);
			Reemitter_->Connect (tc);
			Others_ << tc;
			emit addNewTab (tr ("Summary"), tc);
			emit changeTabIcon (tc,
					QIcon (":/resources/images/leechcraft.svg"));

			tc->SetQuery (query);
		}
	}

	void TabContentsManager::RemoveTab (TabContents *tc)
	{
		emit removeTab (tc);
		Others_.removeAll (tc);
		delete tc;
	}

	void TabContentsManager::MadeCurrent (TabContents *tc)
	{
		Current_ = tc;

		Q_FOREACH (TabContents *tab, GetTabs ())
			if (tab != tc)
				tab->SmartDeselect (tc);

		if (tc)
		{
			QItemSelectionModel *selectionModel = tc->
				GetUi ().PluginsTasksTree_->selectionModel ();
			QItemSelection sel;
			if (selectionModel)
				sel = selectionModel->selection ();

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
	}

	TabContents* TabContentsManager::GetCurrent () const
	{
		return Current_;
	}

	QObject* TabContentsManager::GetReemitter () const
	{
		return Reemitter_;
	}

	void TabContentsManager::Connect (TabContents *tc)
	{
		connect (tc,
				SIGNAL (filterUpdated ()),
				this,
				SLOT (handleFilterUpdated ()));
	}

	void TabContentsManager::handleFilterUpdated ()
	{
		TabContents *tc = qobject_cast<TabContents*> (sender ());
		if (!tc)
			return;

		Reemitter_->ConnectModelSpecific (tc);
	}
};

