#ifndef NOTIFICATION_H
#define NOTIFICATION_H
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

