#include "fileexistsdialog.h"

FileExistsDialog::FileExistsDialog (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
}

FileExistsDialog::Result FileExistsDialog::GetSelected () const
{
	switch (WhatToDo->currentIndex ())
	{
		case 0:
			return Continue;
		case 1:
			return Scratch;
		case 2:
			return Unique;
		case 3:
			return Abort;
		default:
			return Unique;
	}
}

