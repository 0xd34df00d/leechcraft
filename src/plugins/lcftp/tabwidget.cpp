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
		};
	};
};

