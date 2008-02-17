#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include "filepicker.h"

FilePicker::FilePicker (QWidget *parent)
: QWidget (parent)
{
    LineEdit_ = new QLineEdit (this);
    BrowseButton_ = new QPushButton (tr ("Browse..."));
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget (LineEdit_);
    lay->addWidget (BrowseButton_);
    setLayout (lay);
    connect (BrowseButton_, SIGNAL (released ()), this, SLOT (chooseFile ()));
    LineEdit_->setMinimumWidth (QApplication::fontMetrics ().width ("thisismaybeadefaultsettingstring,dont"));
}

void FilePicker::SetText (const QString& text)
{
    LineEdit_->setText (text);
}

QString FilePicker::GetText () const
{
    return LineEdit_->text ();
}

void FilePicker::chooseFile ()
{
    QString name = QFileDialog::getExistingDirectory (this, tr ("Select file"), LineEdit_->text (), 0);
    if (name.isEmpty ())
        return;

    LineEdit_->setText (name);
    emit textChanged (name);
}

