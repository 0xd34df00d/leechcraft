#ifndef PLUGINS_LMP_KEYINTERCEPTOR_H
#define PLUGINS_LMP_KEYINTERCEPTOR_H
#include <QObject>

class QEvent;
class QKeyEvent;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class KeyInterceptor : public QObject
			{
				Q_OBJECT
			public:
				KeyInterceptor (QObject* = 0);
				virtual ~KeyInterceptor ();
			protected:
				virtual bool eventFilter (QObject*, QEvent*);

				void keyPressEvent (QKeyEvent*);
				void keyReleaseEvent (QKeyEvent*);
			};
		};
	};
};

#endif

