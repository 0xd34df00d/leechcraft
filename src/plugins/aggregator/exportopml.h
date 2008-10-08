#ifndef EXPORTOPML_H
#define EXPORTOPML_H
#include <QDialog>
#include "ui_exportopml.h"

class ExportOPML : public QDialog
{
	Q_OBJECT

	Ui::ExportOPML Ui_;
public:
	ExportOPML ();
	virtual ~ExportOPML ();

	QString GetDestination () const;
private slots:
	void on_File__textEdited (const QString&);
	void on_Browse__released ();
};

#endif

