#ifndef PLUGINS_POSHUKU_PLUGINS_FUA_SETTINGS_H
#define PLUGINS_POSHUKU_PLUGINS_FUA_SETTINGS_H
#include <QWidget>
#include "ui_settings.h"

class QStandardItemModel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace Fua
				{
					class FUA;

					class Settings : public QWidget
					{
						Q_OBJECT

						Ui::Settings Ui_;
						FUA *Fua_;
						QStandardItemModel *Model_;
					public:
						Settings (QStandardItemModel*, FUA*);
					private slots:
						void on_Add__released ();
						void on_Modify__released ();
						void on_Remove__released ();
					};
				};
			};
		};
	};
};

#endif

