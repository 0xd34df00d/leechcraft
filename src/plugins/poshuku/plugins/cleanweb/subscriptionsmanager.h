#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_SUBSCRIPTIONSMANAGER_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_SUBSCRIPTIONSMANAGER_H
#include <QWidget>
#include "ui_subscriptionsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					class SubscriptionsManager : public QWidget
					{
						Q_OBJECT

						Ui::SubscriptionsManager Ui_;
					public:
						SubscriptionsManager (QWidget* = 0);
					private slots:
						void on_RemoveButton__released ();
					};
				};
			};
		};
	};
};

#endif

