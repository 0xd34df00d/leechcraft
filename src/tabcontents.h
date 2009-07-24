#ifndef TABCONTENTS_H
#define TABCONTENTS_H
#include <QWidget>
#include "ui_tabcontents.h"

class QTimer;

namespace LeechCraft
{
	class TabContents : public QWidget
	{
		Q_OBJECT

		Ui::TabContents Ui_;
		QTimer *FilterTimer_;
	public:
		TabContents (QWidget* = 0);
		virtual ~TabContents ();

		Ui::TabContents GetUi () const;
		void SmartDeselect ();
	private slots:
		void updatePanes (const QItemSelection&, const QItemSelection&);
		void filterParametersChanged ();
		void filterReturnPressed ();
		void feedFilterParameters ();
	};
};

#endif

