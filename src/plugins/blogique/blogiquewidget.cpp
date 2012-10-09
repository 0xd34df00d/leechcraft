/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "blogiquewidget.h"
#include <QWidgetAction>
#include <QComboBox>
#include <boost/concept_check.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <interfaces/itexteditor.h>
#include <interfaces/core/ipluginsmanager.h>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "interfaces/blogique/iblogiquesidewidget.h"
#include "blogique.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
	QObject *BlogiqueWidget::S_ParentMultiTabs_ = 0;

	BlogiqueWidget::BlogiqueWidget (QWidget *parent)
	: QWidget (parent)
	, PostEdit_ (0)
	, PostEditWidget_ (0)
	, ToolBar_ (new QToolBar)
	, PrevAccountId_ (0)
	{
		Ui_.setupUi (this);
		auto plugs = Core::Instance ().GetCoreProxy ()->
				GetPluginsManager ()->GetAllCastableTo<ITextEditor*> ();

		QVBoxLayout *editFrameLay = new QVBoxLayout ();
		editFrameLay->setContentsMargins (0, 0, 0, 0);
		Ui_.PostFrame_->setLayout (editFrameLay);
		Q_FOREACH (ITextEditor *plug, plugs)
		{
			if (!plug->SupportsEditor (ContentType::PlainText))
				continue;

			QWidget *w = plug->GetTextEditor (ContentType::PlainText);
			PostEdit_ = qobject_cast<IEditorWidget*> (w);
			if (!PostEdit_)
			{
				delete w;
				continue;
			}

			PostEditWidget_ = w;
			editFrameLay->addWidget (w);
			break;
		}

		Ui_.SaveEntry_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("document-save"));
		ToolBar_->addAction (Ui_.SaveEntry_);
		Ui_.Submit_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("svn-commit"));
		ToolBar_->addAction (Ui_.Submit_);

		AccountsBox_ = new QComboBox (ToolBar_);
		AccountsBox_->addItem (QString ());
		connect (AccountsBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleCurrentAccountChanged (int)));
		for (IAccount *acc : Core::Instance ().GetAccounts ())
		{
			AccountsBox_->addItem (acc->GetAccountName ());
			Id2Account_ [AccountsBox_->count () - 1] = acc;
		}
		QWidgetAction *action = new QWidgetAction (ToolBar_);
		action->setDefaultWidget (AccountsBox_);
		ToolBar_->addAction (action);

		Ui_.OpenInBrowser_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("applications-internet"));
		ToolBar_->addAction (Ui_.OpenInBrowser_);

		connect (Ui_.SaveEntry_,
				SIGNAL (triggered ()),
				this,
				SLOT (saveEntry ()));
		connect (Ui_.Submit_,
				SIGNAL (triggered ()),
				this,
				SLOT (submit ()));

		if (!Ui_.MainSplitter_->restoreState (XmlSettingsManager::Instance ()
				.property ("MainSplitterPosition").toByteArray ()))
		{
			Ui_.MainSplitter_->setStretchFactor (0, 6);
			Ui_.MainSplitter_->setStretchFactor (1, 1);
		}

		if (!Ui_.CalendarSplitter_->restoreState (XmlSettingsManager::Instance ()
				.property ("CalendarSplitterPosition").toByteArray ()))
		{
			Ui_.CalendarSplitter_->setStretchFactor (0, 1);
			Ui_.CalendarSplitter_->setStretchFactor (1, 4);
		}

		connect (Ui_.MainSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));
		connect (Ui_.CalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));
	}

	QObject* BlogiqueWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	TabClassInfo BlogiqueWidget::GetTabClassInfo () const
	{
		return qobject_cast<Plugin*> (S_ParentMultiTabs_)->
				GetTabClasses ().first ();
	}

	QToolBar* BlogiqueWidget::GetToolBar () const
	{
		return ToolBar_;
	}

	void BlogiqueWidget::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	void BlogiqueWidget::SetParentMultiTabs (QObject *tab)
	{
		S_ParentMultiTabs_ = tab;
	}

	void BlogiqueWidget::handleCurrentAccountChanged (int id)
	{
		if (PrevAccountId_)
		{
			auto ibp = qobject_cast<IBloggingPlatform*> (Id2Account_ [PrevAccountId_]->
					GetParentBloggingPlatform ());
			for (auto action : ibp->GetEditorActions ())
				PostEdit_->RemoveAction (action);

			for (auto w : SidePluginsWidgets_)
				w->deleteLater ();
			SidePluginsWidgets_.clear ();
		}

		PrevAccountId_ = id;
		if (!PrevAccountId_)
			return;

		auto ibp = qobject_cast<IBloggingPlatform*> (Id2Account_ [PrevAccountId_]->
				GetParentBloggingPlatform ());
		for (auto action : ibp->GetEditorActions ())
			PostEdit_->AppendAction (action);

		for (auto w : ibp->GetBlogiqueSideWidgets ())
		{
			IBlogiqueSideWidget *ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
			{
				qWarning () << Q_FUNC_INFO
						<< "Side widget"
						<< w
						<< "from"
						<< ibp
						<< "is not an IBlogiqueSideWidget";
				continue;
			}

			SidePluginsWidgets_ << w;
			ibsw->SetAccount (Id2Account_ [id]->GetObject ());
			Ui_.Tools_->addItem (w, ibsw->GetName ());
		}
	}

	void BlogiqueWidget::saveEntry ()
	{

	}

	void BlogiqueWidget::submit ()
	{
		if (!AccountsBox_->currentIndex ())
			return;

		QVariantMap postOptions, customData;
		for (auto w : SidePluginsWidgets_)
		{
			auto ibsw = qobject_cast<IBlogiqueSideWidget*> (w);
			if (!ibsw)
				continue;

			switch (ibsw->GetWidgetType ())
			{
			case SideWidgetType::PostOptionsSideWidget:
				postOptions.unite (ibsw->GetPostOptions ());
			case SideWidgetType::CustomSideWidget:
				customData.unite (ibsw->GetCustomData ());
			}
		}

		IAccount *acc = Id2Account_.value (AccountsBox_->currentIndex ());
		if (!acc)
			return;

		Event e;
		e.Content_ = PostEdit_->GetContents (ContentType::HTML);
		e.Subject_ = Ui_.Subject_->text ();
		e.PostOptions_ = postOptions;
		e.CustomData_ = customData;
		acc->submit (e);
	}

	void BlogiqueWidget::saveSplitterPosition (int, int)
	{
		XmlSettingsManager::Instance ().setProperty ("MainSplitterPosition",
				Ui_.MainSplitter_->saveState ());
		XmlSettingsManager::Instance ().setProperty ("CalendarSplitterPosition",
				Ui_.CalendarSplitter_->saveState ());
	}

}
}

