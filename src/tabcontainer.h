#ifndef TABCONTAINER_H
#define TABCONTAINER_H
#include <QObject>
#include <QStringList>

class QIcon;
class QKeyEvent;

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
	public:
		TabContainer (TabWidget*, QObject* = 0);
		virtual ~TabContainer ();

		QWidget* GetWidget (int) const;
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
	private:
		int FindTabForWidget (QWidget*) const;
	};
};

#endif

