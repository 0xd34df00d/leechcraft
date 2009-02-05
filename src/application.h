#ifndef APPLICATION_H
#define APPLICATION_H
#include <QApplication>

namespace LeechCraft
{
	class Application : public QApplication
	{
		Q_OBJECT
	public:
		Application (int&, char**);
		virtual bool notify (QObject*, QEvent*);
	};
};

#endif

