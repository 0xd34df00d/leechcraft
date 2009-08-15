#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#include <QDialog>
#include "ui_aboutdialog.h"

namespace LeechCraft
{
	class AboutDialog : public QDialog
	{
		Q_OBJECT

		Ui::AboutDialog Ui_;
	public:
		AboutDialog (QWidget* = 0);
	};
};

#endif

