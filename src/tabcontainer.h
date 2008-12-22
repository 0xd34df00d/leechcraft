#ifndef TABCONTAINER_H
#define TABCONTAINER_H
#include <QObject>

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
	public:
		TabContainer (QTabWidget*, QObject* = 0);
		virtual ~TabContainer ();

		void Add (QWidget*, const QString&);
		void Remove (QWidget*);
		void ChangeTabName (QWidget*, const QString&);
		void ChangeTabIcon (QWidget*, const QIcon&);
		QWidget* GetWidget (int) const;
		void BringToFront (QWidget*) const;
		bool RemoveCurrent ();
		void RotateLeft ();
		void RotateRight ();
		void SetTabMode (bool);
	private:
		int FindTabForWidget (QWidget*) const;
	};
};

#endif

