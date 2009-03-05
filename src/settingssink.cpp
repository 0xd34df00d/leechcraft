#include "settingssink.h"
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <QtDebug>

using namespace LeechCraft;

LeechCraft::SettingsSink::SettingsSink (const QString& name,
		Util::XmlSettingsDialog* dialog,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	// Because Qt Designer inserts one.
	Ui_.Dialogs_->removeWidget (Ui_.Dialogs_->currentWidget ());

	Add (name, windowIcon (), dialog);
}

LeechCraft::SettingsSink::~SettingsSink ()
{
}

void LeechCraft::SettingsSink::AddDialog (const QObject *object)
{
	IInfo *info = qobject_cast<IInfo*> (object);
	IHaveSettings *ihs = qobject_cast<IHaveSettings*> (object);

	Add (info->GetName (), info->GetIcon (), ihs->GetSettingsDialog ().get ());

	Ui_.Combobox_->setCurrentIndex (0);
}

void LeechCraft::SettingsSink::Add (const QString& name, const QIcon& wicon,
		QWidget *widget)
{
	Ui_.Combobox_->addItem (wicon, name);
	Ui_.Dialogs_->addWidget (widget);
	adjustSize ();

	connect (this,
			SIGNAL (accepted ()),
			widget,
			SLOT (accept ()));
	connect (this,
			SIGNAL (rejected ()),
			widget,
			SLOT (reject ()));
}

