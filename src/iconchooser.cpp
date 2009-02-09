#include "iconchooser.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

IconChooser::IconChooser (const QStringList& sets, QWidget *parent)
: QComboBox (parent)
, Sets_ (sets)
{
	addItems (sets);
	int index = sets.indexOf (XmlSettingsManager::Instance ()->
			Property ("IconSet", "oxygen").toString ());
	setCurrentIndex (index == -1 ? 0 : index);
}

void IconChooser::accept ()
{
	XmlSettingsManager::Instance ()->
		setProperty ("IconSet", currentText ());
	emit requestNewIconSet ();
}

void IconChooser::reject ()
{
	int index = Sets_.indexOf (XmlSettingsManager::Instance ()->
			Property ("IconSet", "oxygen").toString ());
	setCurrentIndex (index == -1 ? 0 : index);
}

