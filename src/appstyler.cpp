#include "appstyler.h"
#include <QStyleFactory>
#include <QApplication>
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

AppStyler::AppStyler (QWidget *parent)
: QComboBox (parent)
{
	addItems (QStyleFactory::keys ());
	reject ();
}

void AppStyler::accept ()
{
	XmlSettingsManager::Instance ()->
		setProperty ("AppQStyle", currentText ());
	QApplication::setStyle (currentText ());
}

void AppStyler::reject ()
{
	int index = findText (XmlSettingsManager::Instance ()->
			Property ("AppQStyle", "Plastique").toString ());
	setCurrentIndex (index == -1 ? 0 : index);
}


