#ifndef TABWIDGET_H
#define TABWIDGET_H
#include <QTabWidget>

namespace LeechCraft
{
	class TabWidget : public QTabWidget
	{
		Q_OBJECT

		bool AsResult_;
	public:
		TabWidget (QWidget* = 0);
	private slots:
		void checkTabMoveAllowed (int, int);
	};
};

#endif

