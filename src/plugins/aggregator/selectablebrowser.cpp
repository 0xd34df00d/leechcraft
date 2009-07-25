#include "selectablebrowser.h"
#include <QVBoxLayout>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			SelectableBrowser::SelectableBrowser (QWidget *parent)
			: QWidget (parent)
			, Internal_ (true)
			{
				QVBoxLayout *lay = new QVBoxLayout ();
				lay->setContentsMargins (0, 0, 0, 0);
				setLayout (lay);
			}
			
			void SelectableBrowser::Construct (IWebBrowser *browser)
			{
				if (browser)
				{
					Internal_ = false;
					InternalBrowser_.reset ();
					ExternalBrowser_.reset (browser->GetWidget ());
					layout ()->addWidget (ExternalBrowser_->Widget ());
				}
				else
				{
					Internal_ = true;
					InternalBrowser_.reset (new QTextBrowser (this));
					ExternalBrowser_.reset ();
					layout ()->addWidget (InternalBrowser_.get ());
				}
			}
			
			void SelectableBrowser::SetHtml (const QString& html, const QUrl& base)
			{
				if (Internal_)
					InternalBrowser_->setHtml (html);
				else
					ExternalBrowser_->SetHtml (html, base);
			}
		};
	};
};

