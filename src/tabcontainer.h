#ifndef TABCONTAINER_H
#define TABCONTAINER_H
#include <QObject>
#include <QStringList>

class QTabWidget;
class QIcon;
class QKeyEvent;

namespace LeechCraft
{
	class TabContainer : public QObject
	{
		Q_OBJECT

		QTabWidget *TabWidget_;
		bool TabMode_;
		QList<QWidget*> Widgets_;
		QStringList TabNames_;
	public:
		TabContainer (QTabWidget*, QObject* = 0);
		virtual ~TabContainer ();

		QWidget* GetWidget (int) const;
		bool RemoveCurrent ();
		void RotateLeft ();
		void RotateRight ();
		void ToggleMultiwindow ();
	public slots:
		void add (const QString&, QWidget*);
		void add (const QString&, QWidget*,
				const QIcon& icon);
		void remove (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void bringToFront (QWidget*) const;
		void handleTabNames ();
	private:
		int FindTabForWidget (QWidget*) const;
	};
};

#endif

