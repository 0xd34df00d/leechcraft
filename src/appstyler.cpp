#include "appstyler.h"
#include <QStyleFactory>
#include <QApplication>
#include "xmlsettingsmanager.h"

#ifdef Q_WS_WIN
#include <QWindowsXPStyle>
#endif

namespace LeechCraft
{
	AppStyler::AppStyler (QWidget *parent)
	: QComboBox (parent)
	{
		addItems (QStyleFactory::keys ());
		reject ();
	}

	void AppStyler::accept ()
	{
		QString style = currentText ();
		XmlSettingsManager::Instance ()->
			setProperty ("AppQStyle", style);
		QApplication::setStyle (style);
	}

	void AppStyler::reject ()
	{
		QString style = XmlSettingsManager::Instance ()->
				property ("AppQStyle").toString ();
		if (style.isEmpty ())
		{
#ifdef Q_WS_WIN
			style = "Plastique";
			XmlSettingsManager::Instance ()->
				setProperty ("AppQStyle", style);
#endif
		}
		QApplication::setStyle (style);
		int index = findText (style);
		setCurrentIndex (index);
	}
};

