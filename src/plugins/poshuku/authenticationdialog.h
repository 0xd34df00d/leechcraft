#ifndef AUTHENTICATIONDIALOG_H
#define AUTHENTICATIONDIALOG_H
#include <QDialog>
#include "ui_authenticationdialog.h"

class AuthenticationDialog : public QDialog
{
	Q_OBJECT

	Ui::AuthenticationDialog Ui_;
public:
	AuthenticationDialog (const QString&,
			const QString&,
			const QString&,
			QWidget* = 0);
	virtual ~AuthenticationDialog ();

	QString GetLogin () const;
	QString GetPassword () const;
};

#endif

