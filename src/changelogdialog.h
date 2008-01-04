#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H
#include <QDialog>
#include "ui_changelogdialog.h"

class ChangelogDialog : public QDialog, private Ui::ChangelogDialog
{
	Q_OBJECT
public:
	ChangelogDialog (QWidget *parent = 0);
};

#endif

