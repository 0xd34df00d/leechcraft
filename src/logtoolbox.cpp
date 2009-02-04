#include "logtoolbox.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

LogToolBox::LogToolBox (QWidget *parent)
: QDialog (parent, Qt::Tool)
{
	Ui_.setupUi (this);

	XmlSettingsManager::Instance ()->RegisterObject ("MaxLogLines",
			this, "handleMaxLogLines");
	handleMaxLogLines ();
}

LogToolBox::~LogToolBox ()
{
}

void LogToolBox::log (const QString& message)
{
	Ui_.Logger_->append (message.trimmed ());
}

void LogToolBox::handleMaxLogLines ()
{
	Ui_.Logger_->document ()->
		setMaximumBlockCount (XmlSettingsManager::Instance ()->
				property ("MaxLogLines").toInt ());
}

void LogToolBox::on_Clear__released ()
{
	Ui_.Logger_->clear ();
}

