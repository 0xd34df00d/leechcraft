#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include "filepickerwidget.h"

FilePickerWidget::FilePickerWidget (QWidget *parent)
: QWidget (parent)
{
	QHBoxLayout *lay = new QHBoxLayout;
	setLayout (lay);

	Browse_ = new QPushButton;
	File_ = new QLineEdit;

	Browse_->setText (tr ("Browse..."));

	lay->addWidget (File_);
	lay->addWidget (Browse_);

	connect (Browse_, SIGNAL (clicked ()), this, SLOT (initiateFileHandling ()));
	connect (File_, SIGNAL (textChanged (const QString&)), this, SIGNAL (textChanged (const QString&)));
}

void FilePickerWidget::setText (const QString& text)
{
	File_->setText (text);
}

QString FilePickerWidget::text () const
{
	return File_->text ();
}

void FilePickerWidget::initiateFileHandling ()
{
	File_->setText (QFileDialog::getExistingDirectory (this, tr ("Select directory")));
}


