/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selectablebrowser.h"
#include <QVBoxLayout>
#include <util/sll/visitor.h>

namespace LC::Util
{
	SelectableBrowser::SelectableBrowser (QWidget *parent)
	: QWidget (parent)
	{
		auto lay = new QVBoxLayout;
		lay->setContentsMargins (0, 0, 0, 0);
		setLayout (lay);
		setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		PrepareInternal ();
	}

	void SelectableBrowser::Construct (IWebBrowser *browser)
	{
		std::unique_ptr<IWebWidget> external;
		if (browser)
			external = browser->CreateWidget ();

		if (external)
		{
			layout ()->addWidget (external->GetQWidget ());
			Browser_ = std::move (external);
		}
		else
			PrepareInternal ();
	}

	void SelectableBrowser::SetHtml (const QString& html, const QUrl& base)
	{
		Util::Visit (Browser_,
				[&] (IWebWidget_ptr& browser) { browser->SetHtml (html, base); },
				[&] (QTextBrowser_ptr& browser) { browser->setHtml (html); });
	}

	void SelectableBrowser::SetNavBarVisible (bool visible)
	{
		Util::Visit (Browser_,
				[&] (IWebWidget_ptr& browser) { browser->SetNavBarVisible (visible); },
				[&] (QTextBrowser_ptr&) {});
	}

	void SelectableBrowser::SetEverythingElseVisible (bool visible)
	{
		Util::Visit (Browser_,
				[&] (IWebWidget_ptr& browser) { browser->SetEverythingElseVisible (visible); },
				[&] (QTextBrowser_ptr&) {});
	}

	void SelectableBrowser::PrepareInternal ()
	{
		auto browser = std::make_unique<QTextBrowser> ();
		browser->setOpenExternalLinks (true);
		browser->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
		layout ()->addWidget (browser.get ());

		Browser_ = std::move (browser);
	}
}
