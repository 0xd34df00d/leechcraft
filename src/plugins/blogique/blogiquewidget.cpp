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
#include <interfaces/itexteditor.h>
#include <interfaces/core/ipluginsmanager.h>
#include "blogique.h"
#include "core.h"
#include "interfaces/blogique/iaccount.h"
#include <QWidgetAction>

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
		for (IAccount *acc : Core::Instance ().GetAccounts ())
			AccountsBox_->addItem (acc->GetAccountName ());
		QWidgetAction *action = new QWidgetAction (ToolBar_);
		action->setDefaultWidget (AccountsBox_);
		ToolBar_->addAction (action);

		Ui_.OpenInBrowser_->setIcon (Core::Instance ()
				.GetCoreProxy ()->GetIcon ("applications-internet"));
		ToolBar_->addAction (Ui_.OpenInBrowser_);
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

}
}

