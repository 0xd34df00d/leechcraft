#ifndef COOKIESEDITDIALOG_H
#define COOKIESEDITDIALOG_H
#include <QDialog>
#include "ui_cookieseditdialog.h"

class CookiesEditModel;

class CookiesEditDialog : public QDialog
{
	Q_OBJECT
	
	Ui::CookiesEditDialog Ui_;
	CookiesEditModel *Model_;
public:
	CookiesEditDialog (QWidget* = 0);
private slots:
	void handleClicked (const QModelIndex&);
	void handleAccepted ();
	void handleDomainChanged ();
	void handleNameChanged ();
	void on_Delete__released ();
};

#endif

