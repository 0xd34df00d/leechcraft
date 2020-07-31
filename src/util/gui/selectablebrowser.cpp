/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selectablebrowser.h"
#include <QVBoxLayout>

namespace LC
{
	namespace Util
	{
		SelectableBrowser::SelectableBrowser (QWidget *parent)
		: QWidget (parent)
		, Internal_ (true)
		{
			QVBoxLayout *lay = new QVBoxLayout ();
			lay->setContentsMargins (0, 0, 0, 0);
			setLayout (lay);
			setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

			PrepareInternal ();
		}

		void SelectableBrowser::Construct (IWebBrowser *browser)
		{
			if (browser &&
					browser->GetWidget ())
			{
				Internal_ = false;
				InternalBrowser_.reset ();
				ExternalBrowser_.reset (browser->GetWidget ());
				layout ()->addWidget (ExternalBrowser_->Widget ());
			}
			else
			{
				ExternalBrowser_.reset ();
				PrepareInternal ();
			}
		}

		void SelectableBrowser::SetHtml (const QString& html, const QUrl& base)
		{
			if (Internal_)
				InternalBrowser_->setHtml (html);
			else
				ExternalBrowser_->SetHtml (html, base);
		}

		void SelectableBrowser::SetNavBarVisible (bool visible)
		{
			if (!Internal_)
				ExternalBrowser_->SetNavBarVisible (visible);
		}

		void SelectableBrowser::SetEverythingElseVisible (bool visible)
		{
			if (!Internal_)
				ExternalBrowser_->SetEverythingElseVisible (visible);
		}

		void SelectableBrowser::PrepareInternal ()
		{
			Internal_ = true;
			InternalBrowser_.reset (new QTextBrowser (this));
			InternalBrowser_->setOpenExternalLinks (true);
			InternalBrowser_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
			layout ()->addWidget (InternalBrowser_.get ());
		}
	}
}
