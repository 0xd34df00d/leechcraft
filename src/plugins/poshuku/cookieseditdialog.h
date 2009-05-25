#ifndef COOKIESEDITDIALOG_H
#define COOKIESEDITDIALOG_H
#include <QDialog>
#include "ui_cookieseditdialog.h"

class CookiesEditModel;
class CookiesFilter;

class CookiesEditDialog : public QDialog
{
	Q_OBJECT
	
	Ui::CookiesEditDialog Ui_;
	CookiesEditModel *Model_;
	CookiesFilter *Filter_;
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

