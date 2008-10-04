#ifndef IMPORTOPML_H
#define IMPORTOPML_H
#include <QDialog>
#include "ui_importopml.h"

class ImportOPML : public QDialog
{
	Q_OBJECT

	Ui::ImportOPML Ui_;
public:
	ImportOPML (QWidget* = 0);
	virtual ~ImportOPML ();

	QString GetFilename () const;
	QString GetTags () const;
	std::vector<bool> GetMask () const;
private slots:
	void on_File__textEdited (const QString&);
	void on_Browse__released ();
private:
	bool HandleFile (const QString&);
	void Reset ();
};

#endif

