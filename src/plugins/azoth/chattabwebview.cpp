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

#include "chattabwebview.h"
#include <QContextMenuEvent>
#include <QWebHitTestResult>
#include <QPointer>
#include <QMenu>
#include <QDesktopServices>
#include <util/util.h>
#include <interfaces/idatafilter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "actionsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	ChatTabWebView::ChatTabWebView (QWidget *parent)
	: QWebView (parent)
	, QuoteAct_ (0)
	{
	}

	void ChatTabWebView::SetQuoteAction (QAction *act)
	{
		QuoteAct_ = act;
	}

	namespace
	{
		struct DataFilterActionInfo
		{
			Entity Entity_;
			QObject *Plugin_;
			QByteArray VarID_;
		};
	}
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::DataFilterActionInfo)

namespace LeechCraft
{
namespace Azoth
{
	void ChatTabWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		QPointer<QMenu> menu (new QMenu (this));
		const auto r = page ()->mainFrame ()->hitTestContent (e->pos ());

		if (!r.linkUrl ().isEmpty ())
		{
			if (r.linkUrl ().scheme () == "azoth")
				HandleNick (menu, r.linkUrl ());
			else
				HandleURL (menu, r.linkUrl ());
		}

		const auto& text = page ()->selectedText ();
		if (!text.isEmpty ())
		{
			menu->addAction (pageAction (QWebPage::Copy));
			menu->addAction (QuoteAct_);
			menu->addSeparator ();

			HandleDataFilters (menu, text);
		}

		if (!r.imageUrl ().isEmpty ())
			menu->addAction (pageAction (QWebPage::CopyImageToClipboard));

		if (settings ()->testAttribute (QWebSettings::DeveloperExtrasEnabled))
			menu->addAction (pageAction (QWebPage::InspectElement));

		if (menu->isEmpty ())
		{
			delete menu;
			return;
		}

		menu->exec (mapToGlobal (e->pos ()));
		if (menu)
			delete menu;
	}

	void ChatTabWebView::HandleNick (QMenu *menu, const QUrl& nickUrl)
	{
		const QString& entryId = QUrl::fromPercentEncoding (nickUrl.queryItemValue ("entryId").toUtf8 ());
		if (entryId.isEmpty ())
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (entryId));
		if (!entry)
			return;

		QList<QAction*> actions;

		ActionsManager *manager = Core::Instance ().GetActionsManager ();
		QList<QAction*> allActions = manager->GetEntryActions (entry);
		Q_FOREACH (QAction *act, allActions)
			if (manager->GetAreasForAction (act)
					.contains (ActionsManager::CLEAAChatCtxtMenu))
				actions << act;

		menu->addActions (actions);
	}

	void ChatTabWebView::HandleURL (QMenu *menu, const QUrl& url)
	{
		menu->addAction (tr ("Open"),
				this,
				SLOT (handleOpenLink ()))->setData (url);
		menu->addAction (tr ("Save..."),
				this,
				SLOT (handleSaveLink ()))->setData (url);
		menu->addAction (tr ("Open externally"),
				this,
				SLOT (handleOpenExternally ()))->setData (url);
		menu->addAction (pageAction (QWebPage::CopyLinkToClipboard));
		menu->addSeparator ();
	}

	void ChatTabWebView::HandleDataFilters (QMenu *menu, const QString& text)
	{
		auto entity = Util::MakeEntity (text,
				QString (),
				static_cast<TaskParameters> (FromUserInitiated) | OnlyHandle,
				"x-leechcraft/data-filter-request");
		auto em	= Core::Instance ().GetProxy ()->GetEntityManager ();
		for (auto plugin : em->GetPossibleHandlers (entity))
		{
			auto ii = qobject_cast<IInfo*> (plugin);
			auto idf = qobject_cast<IDataFilter*> (plugin);
			const auto& vars = idf->GetFilterVariants ();
			if (!vars.isEmpty ())
			{
				auto searchMenu = menu->addMenu (ii->GetIcon (), idf->GetFilterVerb ());
				searchMenu->menuAction ()->setIcon (ii->GetIcon ());
				for (const auto& var : vars)
				{
					auto act = searchMenu->addAction (var.Name_);
					const DataFilterActionInfo info
					{
						entity,
						plugin,
						var.ID_
					};
					act->setData (QVariant::fromValue (info));
					connect (act,
							SIGNAL (triggered ()),
							this,
							SLOT (handleDataFilterAction ()));
				}
			}
			else
			{
				auto searchAct = menu->addAction (ii->GetIcon (), idf->GetFilterVerb ());
				const DataFilterActionInfo info
				{
					entity,
					plugin,
					QByteArray ()
				};
				searchAct->setData (QVariant::fromValue (info));
				connect (searchAct,
						SIGNAL (triggered ()),
						this,
						SLOT (handleDataFilterAction ()));
			}
		}
	}

	void ChatTabWebView::handleOpenLink ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		const Entity& e = Util::MakeEntity (action->data (),
				QString (),
				static_cast<TaskParameters> (OnlyHandle | FromUserInitiated));
		Core::Instance ().SendEntity (e);
	}

	void ChatTabWebView::handleOpenExternally ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		const QUrl& url = action->data ().toUrl ();
		if (url.isEmpty ())
			return;

		QDesktopServices::openUrl (url);
	}

	void ChatTabWebView::handleSaveLink ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		Entity e = Util::MakeEntity (action->data (),
				QString (),
				FromUserInitiated);
		e.Additional_ ["AllowedSemantics"] = QStringList ("fetch") << "save";
		Core::Instance ().SendEntity (e);
	}

	void ChatTabWebView::handleDataFilterAction ()
	{
		auto action = qobject_cast<QAction*> (sender ());
		const auto& data = action->data ().value<DataFilterActionInfo> ();

		auto entity = data.Entity_;
		entity.Additional_ ["DataFilter"] = data.VarID_;
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (entity, data.Plugin_);
	}
}
}
