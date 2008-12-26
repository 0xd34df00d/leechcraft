#ifndef FINDDIALOG_H
#define FINDDIALOG_H
#include <QDialog>
#include <QWebPage>
#include "ui_finddialog.h"

class FindDialog : public QDialog
{
	Q_OBJECT
	
	Ui::FindDialog Ui_;
public:
	FindDialog (QWidget* = 0);
	virtual ~FindDialog ();

	void SetSuccessful (bool);
private slots:
	void on_Pattern__textChanged (const QString&);
	void on_FindButton__released ();
signals:
	void next (const QString&, QWebPage::FindFlags);
};

#endif

