#ifndef TABCONTAINER_H
#define TABCONTAINER_H
#include <QObject>
#include <QStringList>
#include <QMap>

class QIcon;
class QKeyEvent;
class QToolBar;

namespace LeechCraft
{
	class TabWidget;

	class TabContainer : public QObject
	{
		Q_OBJECT

		TabWidget *TabWidget_;
		bool TabMode_;
		QList<QWidget*> Widgets_;
		QStringList TabNames_;
		QList<QKeyEvent*> Events_;
		QMap<QWidget*, QToolBar*> StaticBars_;
	public:
		TabContainer (TabWidget*, QObject* = 0);
		virtual ~TabContainer ();

		QWidget* GetWidget (int) const;
		QToolBar* GetToolBar (int) const;
		void SetToolBar (QToolBar*, QWidget*);
		void RotateLeft ();
		void RotateRight ();
		void ToggleMultiwindow ();
		void ForwardKeyboard (QKeyEvent*);
	public slots:
		void add (const QString&, QWidget*);
		void add (const QString&, QWidget*,
				const QIcon& icon);
		void remove (QWidget*);
		void remove (int);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void bringToFront (QWidget*) const;
		void handleTabNames ();
		void handleScrollButtons ();
	private:
		int FindTabForWidget (QWidget*) const;
	};
};

#endif

