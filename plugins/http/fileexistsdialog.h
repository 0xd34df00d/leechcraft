#ifndef FILEEXISTSDIALOG_H
#define FILEEXISTSDIALOG_H
#include "ui_fileexistsdialog.h"

class FileExistsDialog : public QDialog, private Ui::FileExistsDialog
{
	Q_OBJECT

public:
	FileExistsDialog (QWidget *parent = 0);

	enum Result { Scratch, Continue, Unique, Abort };

	Result GetSelected () const;
};

#endif

