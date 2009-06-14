#ifndef PLUGINS_POSHUKU_NOTIFICATION_H
#define PLUGINS_POSHUKU_NOTIFICATION_H
#include <QWidget>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class Notification : public QWidget
			{
				Q_OBJECT
			public:
				Notification (QWidget* = 0);
			};
		};
	};
};

#endif

