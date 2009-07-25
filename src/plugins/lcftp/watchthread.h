#ifndef PLUGINS_LCFTP_WATCHTHREAD_H
#define PLUGINS_LCFTP_WATCHTHREAD_H
#include <QThread>
#include <plugininterface/guarded.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class WatchThread : public QThread
			{
				Q_OBJECT

				Util::Guarded<bool> Quitting_;
			public:
				WatchThread (QObject* = 0);
				void SetExit ();
			protected:
				void run ();
			signals:
				void shouldPerform ();
			};
		};
	};
};

#endif

