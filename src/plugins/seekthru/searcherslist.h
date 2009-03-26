#ifndef SEARCHERSLIST_H
#define SEARCHERSLIST_H
#include <QWidget>
#include "ui_searcherslist.h"

class SearchersList : public QWidget
{
	Q_OBJECT

	Ui::SearchersList Ui_;
	QModelIndex Current_;
public:
	SearchersList (QWidget* = 0);
private slots:
	void handleCurrentChanged (const QModelIndex&);
	void on_ButtonAdd__released ();
	void on_ButtonRemove__released ();
	void on_Tags__textEdited (const QString&);
};

#endif

