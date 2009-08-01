#ifndef TABCONTENTS_H
#define TABCONTENTS_H
#include <QWidget>
#include "ui_tabcontents.h"

class QTimer;
class QToolBar;

namespace LeechCraft
{
	class TabContents : public QWidget
	{
		Q_OBJECT

		Ui::TabContents Ui_;
		QTimer *FilterTimer_;
		QToolBar *Controls_;
	public:
		TabContents (QWidget* = 0);
		virtual ~TabContents ();

		Ui::TabContents GetUi () const;
		void SmartDeselect (TabContents*);
		void SetQuery (const QString&);
	private slots:
		void updatePanes (const QItemSelection&, const QItemSelection&);
		void filterParametersChanged ();
		void filterReturnPressed ();
		void feedFilterParameters ();
		void on_PluginsTasksTree__customContextMenuRequested (const QPoint&);
	signals:
		void filterUpdated ();
	};
};

#endif

