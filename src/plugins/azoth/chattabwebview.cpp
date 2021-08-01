/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattabwebview.h"
#include <QContextMenuEvent>
#include <QWebHitTestResult>
#include <QPointer>
#include <QMenu>
#include <QDesktopServices>
#include <QUrlQuery>
#include <util/xpc/util.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include <util/sll/util.h>
#include <util/sll/qtutil.h>
#include <interfaces/idatafilter.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "actionsmanager.h"

namespace LC::Azoth
{
	ChatTabWebView::ChatTabWebView (QWidget *parent)
	: QWebView (parent)
	{
		connect (page (),
				&QWebPage::linkClicked,
				this,
				[this] (const QUrl& url) { emit linkClicked (url, true); });
	}

	void ChatTabWebView::SetQuoteAction (QAction *act)
	{
		QuoteAct_ = act;
	}

	void ChatTabWebView::mouseReleaseEvent (QMouseEvent *e)
	{
		if (e->button () != Qt::MiddleButton)
			return QWebView::mouseReleaseEvent (e);

		const auto r = page ()->mainFrame ()->hitTestContent (e->pos ());
		if (r.linkUrl ().isEmpty ())
			return QWebView::mouseReleaseEvent (e);

		emit linkClicked (r.linkUrl (), false);
	}

	void ChatTabWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		QPointer<QMenu> menu (new QMenu (this));
		const auto menuGuard = Util::MakeScopeGuard ([&menu] { delete menu; });

		const auto r = page ()->mainFrame ()->hitTestContent (e->pos ());

		if (!r.linkUrl ().isEmpty ())
		{
			if (r.linkUrl ().scheme () == "azoth"_ql)
				HandleNick (menu, r.linkUrl ());
			else
				HandleURL (menu, r.linkUrl ());
		}

		const auto& text = page ()->selectedText ();
		if (!text.isEmpty ())
		{
			menu->addAction (pageAction (QWebPage::Copy));
			menu->addAction (QuoteAct_);

			if (!text.contains (' ') && text.contains ('.'))
				menu->addAction (tr ("Open as URL"),
						this,
						[text]
						{
							QUrl url { text.trimmed () };
							if (url.scheme ().isEmpty () &&
									url.host ().isEmpty ())
								url = "http://" + url.toString ();

							const auto& e = Util::MakeEntity (url,
									{},
									OnlyHandle | FromUserInitiated);
							Core::Instance ().SendEntity (e);
						});

			menu->addSeparator ();

			menu->addAction (tr ("Highlight all occurrences"),
					this,
					[this, text] { emit chatWindowSearchRequested (text.trimmed ()); });

			menu->addSeparator ();

			new Util::StdDataFilterMenuCreator (text,
					Core::Instance ().GetProxy ()->GetEntityManager (), menu);
		}

		if (!r.imageUrl ().isEmpty ())
			menu->addAction (pageAction (QWebPage::CopyImageToClipboard));

		if (settings ()->testAttribute (QWebSettings::DeveloperExtrasEnabled))
			menu->addAction (pageAction (QWebPage::InspectElement));

		if (menu->isEmpty ())
			return;

		menu->exec (mapToGlobal (e->pos ()));
	}

	void ChatTabWebView::HandleNick (QMenu *menu, const QUrl& nickUrl)
	{
		const auto& entryIdValue = QUrlQuery { nickUrl }.queryItemValue (QStringLiteral ("entryId"));
		const auto& entryId = QUrl::fromPercentEncoding (entryIdValue.toUtf8 ());
		if (entryId.isEmpty ())
			return;

		const auto entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (entryId));
		if (!entry)
			return;

		QList<QAction*> actions;

		const auto manager = Core::Instance ().GetActionsManager ();
		for (const auto act : manager->GetEntryActions (entry))
			if (manager->GetAreasForAction (act).contains (ActionsManager::CLEAAChatCtxtMenu))
				actions << act;

		menu->addActions (actions);

		menu->addAction (tr ("Highlight the nickname"),
				this,
				[this, name = entry->GetEntryName ()] { emit chatWindowSearchRequested (name); });
	}

	void ChatTabWebView::HandleURL (QMenu *menu, const QUrl& url)
	{
		menu->addAction (tr ("Open"),
				this,
				[url]
				{
					const auto& e = Util::MakeEntity (url,
							{},
							OnlyHandle | FromUserInitiated);
					Core::Instance ().SendEntity (e);
				});
		menu->addAction (tr ("Save..."),
				this,
				[url]
				{
					auto e = Util::MakeEntity (url,
							{},
							FromUserInitiated);
					static const QStringList semantics { QStringLiteral ("fetch"), QStringLiteral ("save") };
					e.Additional_ [QStringLiteral ("AllowedSemantics")] = semantics;
					Core::Instance ().SendEntity (e);
				});
		menu->addAction (tr ("Open externally"),
				this,
				[url] { QDesktopServices::openUrl (url); });
		menu->addAction (pageAction (QWebPage::CopyLinkToClipboard));
		menu->addSeparator ();
	}
}
