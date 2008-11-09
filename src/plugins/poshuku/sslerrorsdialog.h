#ifndef SSLERRORSDIALOG_H
#define SSLERRORSDIALOG_H
#include <QDialog>
#include <QList>
#include <QSslError>
#include "ui_sslerrorsdialog.h"

class SslErrorsDialog : public QDialog
{
	Q_OBJECT

	Ui::SslErrorsDialog Ui_;
public:
	SslErrorsDialog (const QString&, const QList<QSslError>&, QWidget* = 0);
	virtual ~SslErrorsDialog ();
private:
	void PopulateTree (const QSslError&);
};

#endif

