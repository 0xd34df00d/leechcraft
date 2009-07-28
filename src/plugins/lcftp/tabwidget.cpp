#include "tabwidget.h"
#include "core.h"
#include "tabmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			TabWidget::TabWidget (const QUrl& url,
					const QString& str,
					QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);

				Setup (Ui_.Left_);
				Setup (Ui_.Right_);

				Ui_.Right_->SetURL (url);
				Ui_.Left_->Navigate (str);
			}

			void TabWidget::Remove ()
			{
				Core::Instance ().GetTabManager ()->Remove (this);
			}

			QToolBar* TabWidget::GetToolBar () const
			{
				return 0;
			}

			void TabWidget::Setup (Pane *p)
			{
				connect (p,
						SIGNAL (downloadRequested (const QUrl&)),
						this,
						SLOT (handleDownloadRequested (const QUrl&)));
			}

			Pane* TabWidget::Other (Pane *p)
			{
				if (p == Ui_.Left_)
					return Ui_.Right_;
				else if (p == Ui_.Right_)
					return Ui_.Left_;
				else
					return 0;
			}

			void TabWidget::handleDownloadRequested (const QUrl& url)
			{
				Pane *other = Other (static_cast<Pane*> (sender ()));
				// TODO we don't support FXP yet
				if (!other->IsLocal ())
					return;
				Core::Instance ().Add (url, other->GetString (), true);
			}
		};
	};
};

