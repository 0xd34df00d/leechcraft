/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urleditbuttonsmanager.h"
#include <QToolButton>
#include <QMenu>
#include <util/sll/slotclosure.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include "interfaces/poshuku/iwebview.h"
#include "core.h"
#include "progresslineedit.h"

namespace LC
{
namespace Poshuku
{
	UrlEditButtonsManager::UrlEditButtonsManager (IWebView *view,
			ProgressLineEdit *edit, QAction *add2favs)
	: QObject { view->GetQWidget () }
	, View_ { view }
	, LineEdit_ { edit }
	, Add2Favorites_ { add2favs }
	, ExternalLinks_ { new QMenu { } }
	, ExternalLinksAction_ { new QAction { this } }
	{
		ExternalLinks_->menuAction ()->setText (tr ("External links"));

		ExternalLinksAction_->setText ("External links");
		ExternalLinksAction_->setProperty ("ActionIcon", "application-rss+xml");

		connect (&Core::Instance (),
				SIGNAL (bookmarkAdded (const QString&)),
				this,
				SLOT (checkPageAsFavorite (const QString&)));
		connect (&Core::Instance (),
				SIGNAL (bookmarkRemoved (const QString&)),
				this,
				SLOT (checkPageAsFavorite (const QString&)));
		connect (LineEdit_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkPageAsFavorite (const QString&)));
		connect (View_->GetQWidget (),
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (updateBookmarksState ()));
		connect (View_->GetQWidget (),
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (checkLinkRels ()));

		LineEdit_->InsertAction (Add2Favorites_, 0, true);

		for (auto action : view->GetActions (IWebView::ActionArea::UrlBar))
		{
			LineEdit_->InsertAction (action, 0, false);
			LineEdit_->SetVisible (action, action->isEnabled ());

			new Util::SlotClosure<Util::NoDeletePolicy>
			{
				[=, this] { LineEdit_->SetVisible (action, action->isEnabled ()); },
				action,
				SIGNAL (changed ()),
				action
			};
		}
	}

	void UrlEditButtonsManager::checkPageAsFavorite (const QString& url)
	{
		if (url != View_->GetUrl ().toString () &&
				url != LineEdit_->text ())
			return;

		if (Core::Instance ().IsUrlInFavourites (url))
		{
			Add2Favorites_->setProperty ("ActionIcon", "list-remove");
			Add2Favorites_->setText (tr ("Remove bookmark"));
			Add2Favorites_->setToolTip (tr ("Remove bookmark"));

			if (auto btn = LineEdit_->GetButtonFromAction (Add2Favorites_))
				btn->setIcon (Core::Instance ().GetProxy ()->
						GetIconThemeManager ()->GetIcon ("list-remove"));
		}
		else
		{
			Add2Favorites_->setProperty ("ActionIcon", "bookmark-new");
			Add2Favorites_->setText (tr ("Add bookmark"));
			Add2Favorites_->setToolTip (tr ("Add bookmark"));

			if (auto btn = LineEdit_->GetButtonFromAction (Add2Favorites_))
				btn->setIcon (Core::Instance ().GetProxy ()->
						GetIconThemeManager ()->GetIcon ("bookmark-new"));
		}
	}

	void UrlEditButtonsManager::checkLinkRels ()
	{
		LineEdit_->RemoveAction (ExternalLinksAction_);

		ExternalLinks_->clear ();
		const auto& mainFrameURL = View_->GetUrl ();

		const auto entityMgr = Core::Instance ().GetProxy ()->GetEntityManager ();
		View_->EvaluateJS (R"(
					(function(){
					var links = document.getElementsByTagName('link');
					var result = [];
					for (var i = 0; i < links.length; ++i)
					{
						var link = links[i];
						result.push({
								"rel": link.rel,
								"type": link.type,
								"href": link.href,
								"title": link.title
							});
					}
					return result;
					})();
				)",
				[=, this] (const QVariant& res)
				{
					bool inserted = false;
					for (const auto& var : res.toList ())
					{
						const auto& map = var.toMap ();

						const auto& type = map ["type"].toString ();
						if (type.isEmpty ())
							continue;

						const auto& title = map ["title"].toString ();
						const auto& href = map ["href"].toString ();
						const auto& rel = map ["rel"].toString ();

						Entity e;
						e.Mime_ = type;

						auto entity = title;
						if (entity.isEmpty ())
						{
							entity = e.Mime_;
							entity.remove ("application/");
							entity.remove ("+xml");
							entity = entity.toUpper ();
						}

						auto entityUrl = mainFrameURL.resolved (QUrl { href });
						e.Entity_ = entityUrl;
						e.Additional_ ["SourceURL"] = entityUrl;
						e.Parameters_ = FromUserInitiated |
										OnlyHandle;
						e.Additional_ ["UserVisibleName"] = entity;
						e.Additional_ ["LinkRel"] = rel;
						e.Additional_ [IgnoreSelf] = true;

						if (entityMgr->CouldHandle (e))
						{
							auto mime = e.Mime_;
							mime.replace ('/', '_');
							const auto& iconStr = QString { ":/resources/images/%1.png" }.arg (mime);
							auto act = ExternalLinks_->addAction (QIcon { iconStr }, entity);

							new Util::SlotClosure<Util::NoDeletePolicy>
									{
											[entityMgr, e] { entityMgr->HandleEntity (e); },
											act,
											SIGNAL (triggered ()),
											act
									};

							if (!inserted)
							{
								auto btn = LineEdit_->InsertAction (ExternalLinksAction_);
								LineEdit_->SetVisible (ExternalLinksAction_, true);
								btn->setMenu (ExternalLinks_.get ());
								btn->setArrowType (Qt::NoArrow);
								btn->setPopupMode (QToolButton::InstantPopup);
								const QString newStyle { "::menu-indicator { image: "
										"url(data:image/gif;base64,R0lGODlhAQABAPABAP///"
										"wAAACH5BAEKAAAALAAAAAABAAEAAAICRAEAOw==);}" };
								btn->setStyleSheet (btn->styleSheet () + newStyle);

								connect (ExternalLinks_->menuAction (),
										SIGNAL (triggered ()),
										this,
										SLOT (showSendersMenu ()),
										Qt::UniqueConnection);
								inserted = true;
							}
						}
					}
				});
	}

	void UrlEditButtonsManager::showSendersMenu ()
	{
		auto action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QAction"
				<< sender ();
			return;
		}

		auto menu = action->menu ();
		menu->exec (QCursor::pos ());
	}

	void UrlEditButtonsManager::updateBookmarksState ()
	{
		checkPageAsFavorite (View_->GetUrl ().toString ());
	}
}
}
