#ifndef TABWIDGET_H
#define TABWIDGET_H
#include <QTabWidget>
#include <QMap>

namespace LeechCraft
{
	class TabWidget : public QTabWidget
	{
		Q_OBJECT

		bool AsResult_;
		QMap<int, QWidget*> Widgets_;
	public:
		TabWidget (QWidget* = 0);
		void SetTooltip (int, QWidget*);
	protected:
		virtual bool event (QEvent*);
		virtual void tabInserted (int);
		virtual void tabRemoved (int);
	private slots:
		void checkTabMoveAllowed (int, int);
	signals:
		void moveHappened (int, int);
	};
};

#endif

