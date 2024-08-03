/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkactionexecutor.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>

namespace LC::Monocle
{
	namespace
	{
		void OpenUrl (const QUrl& url)
		{
			auto e = Util::MakeEntity (url, {}, FromUserInitiated);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		struct LinkActionMenu
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Monocle::LinkActionMenu)
		public:
			static QString GetPrimaryActionLabel (NoAction)
			{
				return {};
			}

			static QString GetPrimaryActionLabel (const NavigationAction& nav)
			{
				return tr ("Navigate to page %1").arg (nav.PageNumber_ + 1);
			}

			static QString GetPrimaryActionLabel (const ExternalNavigationAction& extNav)
			{
				return tr ("Navigate to page %1 of %2")
						.arg (extNav.DocumentNavigation_.PageNumber_ + 1)
						.arg (extNav.TargetDocument_);
			}

			static QString GetPrimaryActionLabel (const UrlAction& url)
			{
				return tr ("Open URL %1").arg (url.Url_.toString ());
			}

			static QString GetPrimaryActionLabel (const CustomAction&)
			{
				return tr ("Execute custom action");
			}
		};
	}

	void ExecuteLinkAction (const LinkAction& action, LinkExecutionContext& ec)
	{
		Util::Visit (action,
				[] (NoAction) {},
				[&] (const NavigationAction& nav) { ec.Navigate (nav); },
				[&] (const ExternalNavigationAction& extNav) { ec.Navigate (extNav); },
				[] (const UrlAction& url) { OpenUrl (url.Url_); },
				[] (const CustomAction& custom) { custom (); });
	}

	void AddLinkMenuActions (const LinkAction& action, QMenu& menu, LinkExecutionContext& ec)
	{
		Util::Visit (action,
				[] (NoAction) {},
				[&] (const NavigationAction& nav)
				{
					auto navigate = menu.addAction (LinkActionMenu::GetPrimaryActionLabel (nav),
							[&ec, nav] { ec.Navigate (nav); });
					navigate->setProperty ("ActionIcon", "quickopen");
				},
				[&] (const ExternalNavigationAction& extNav)
				{
					auto navigate = menu.addAction (LinkActionMenu::GetPrimaryActionLabel (extNav),
							[&ec, extNav] { ec.Navigate (extNav); });
					navigate->setProperty ("ActionIcon", "quickopen-file");

					auto copy = menu.addAction (LinkActionMenu::tr ("Copy the linked document name to the clipboard"),
							[extNav] { QGuiApplication::clipboard ()->setText (extNav.TargetDocument_); });
					copy->setProperty ("ActionIcon", "edit-copy");
				},
				[&] (const UrlAction& url)
				{
					auto open = menu.addAction (LinkActionMenu::GetPrimaryActionLabel (url),
							[url] { OpenUrl (url.Url_); });
					open->setProperty ("ActionIcon", "document-open-remote");

					auto download = menu.addAction (LinkActionMenu::tr ("Download %1").arg (url.Url_.toString ()),
							[url]
							{
								auto e = Util::MakeEntity (url.Url_, {}, FromUserInitiated | OnlyDownload);
								GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
							});
					download->setProperty ("ActionIcon", "download");

					auto copy = menu.addAction (LinkActionMenu::tr ("Copy URL to the clipboard"),
							[url] { QGuiApplication::clipboard ()->setText (url.Url_.toString ()); });
					copy->setProperty ("ActionIcon", "edit-copy");
				},
				[&] (const CustomAction& custom)
				{
					auto exec = menu.addAction (LinkActionMenu::GetPrimaryActionLabel (custom),
							custom);
					exec->setProperty ("ActionIcon", "document-open-remote");
				});
	}

	QString GetLinkActionTooltip (const LinkAction& action)
	{
		return Util::Visit (action,
				[] (const auto& action) { return LinkActionMenu::GetPrimaryActionLabel (action); });
	}
}
