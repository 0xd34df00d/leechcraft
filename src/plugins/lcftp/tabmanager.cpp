#include "tabmanager.h"
#include <QUrl>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			TabManager::TabManager (QObject *parent)
			: QObject (parent)
			{
			}

			TabManager::~TabManager ()
			{
				qDeleteAll (Widgets_);
			}

			void TabManager::AddTab (const QUrl& url, const QString& local)
			{
				TabWidget_ptr w (new TabWidget (url, local));
				emit addNewTab (url.host (), w);
				Widgets_ << w;
			}

			void TabManager::Remove (TabWidget *tab)
			{
				Widgets_.removeAll (tab);
				emit removeTab (tab);
				tab->deleteLater ();
			}
		};
	};
};

