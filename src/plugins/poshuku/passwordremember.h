#ifndef PLUGINS_POSHUKU_PASSWORDREMEMBER_H
#define PLUGINS_POSHUKU_PASSWORDREMEMBER_H
#include "notification.h"
#include "core.h"
#include "pageformsdata.h"
#include "ui_passwordremember.h"

class PasswordRemember : public Notification
{
	Q_OBJECT

	Ui::PasswordRemember Ui_;
	QPair<QString, ElementsData_t> TempData_;
public:
	PasswordRemember (QWidget* = 0);
public slots:
	void add (const PageFormsData_t&);
private slots:
	void on_Remember__released ();
	void on_NotNow__released ();
	void on_Never__released ();
};

#endif

