#ifndef FILEEXISTSDIALOG_H
#define FILEEXISTSDIALOG_H
#include "ui_fileexistsdialog.h"

class FileExistsDialog : public QDialog, private Ui::FileExistsDialog
{
	Q_OBJECT
public:
	enum Result { Scratch, Continue, Unique, Abort };

	FileExistsDialog (QWidget *parent = 0);
	Result GetSelected () const;
};

#endif

