#include "logtoolbox.h"

LogToolBox::LogToolBox (QWidget *parent)
: QDialog (parent, Qt::Tool)
{
	Ui_.setupUi (this);
}

LogToolBox::~LogToolBox ()
{
}

void LogToolBox::log (const QString& message)
{
	Ui_.Logger_->append (message.trimmed ());
}

void LogToolBox::on_Clear__released ()
{
	Ui_.Logger_->clear ();
}

