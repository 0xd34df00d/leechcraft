#include "tabmanager.h"
#include <QUrl>
#include <QDir>
#include "xmlsettingsmanager.h"

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

			void TabManager::AddTab (const QUrl& url, QString local)
			{
				if (local.isEmpty () ||
						local == "." ||
						local == "..")
					local = XmlSettingsManager::Instance ()
						.Property ("LastPanedLocalPath", QDir::homePath ()).toString ();

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

