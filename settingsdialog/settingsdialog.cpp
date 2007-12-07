#include <QVariant>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QtDebug>
#include <QMetaProperty>
#include <QLayout>
#include "settingsdialog.h"
#include "settingsiteminfo.h"
#include "typehandler.h"
#include "../interfaces/interfaces.h"

SettingsDialog::SettingsDialog (QWidget *parent)
: QDialog (parent)
{
	Pages_ = new QStackedWidget (this);
	Combo_ = new QComboBox (this);

	connect (Combo_, SIGNAL (activated (int)), Pages_, SLOT (setCurrentIndex (int)));
	connect (Combo_, SIGNAL (activated (int)), this, SLOT (doAdjustSize ()));

	QVBoxLayout *vl = new QVBoxLayout (this);
	QHBoxLayout *comboContainer = new QHBoxLayout;
	comboContainer->addWidget (Combo_);
	comboContainer->addStretch (1);

	OK_ = new QPushButton (tr ("OK"));
	Cancel_ = new QPushButton (tr ("Cancel"));
	QHBoxLayout *buttons = new QHBoxLayout;
	buttons->addStretch (1);
	buttons->addWidget (OK_);
	buttons->addWidget (Cancel_);
	connect (OK_, SIGNAL (released ()), this, SLOT (accept ()));
	connect (Cancel_, SIGNAL (released ()), this, SLOT (reject ()));

	vl->addLayout (comboContainer);
	vl->addWidget (Pages_);
	vl->addLayout (buttons);

	TypeHandler_ = new TypeHandler (this);
}

SettingsDialog::~SettingsDialog ()
{
	delete TypeHandler_;
}

void SettingsDialog::RegisterObject (QObject *object)
{	
	ISettings *castedObject = qobject_cast<ISettings*> (object);
	if (!castedObject)
	{
		qDebug () << "Registered object isn't true settings manager, ignoring it";
		return;
	}
	// Because append() takes [const T&] where T is QObject*, we won't
	// actually lose any constness in the next line.
	Objects_.append (const_cast<QObject*> (object));

	const QMetaObject *mo = object->metaObject ();

	for (int i = mo->propertyOffset (); i < mo->propertyCount (); ++i)
	{
		QMetaProperty currentProp = mo->property (i);
		QString propName = currentProp.name ();

		SettingsItemInfo sii = castedObject->GetInfoFor (propName);

		QWidget *representator = (*TypeHandler_) (object->property (propName.toAscii ().data ()), sii, propName, object);
		if (!representator)
		{
			qDebug () << "Creating representator widget failed for type" << object->property (propName.toAscii ().data ()) << ", continuing...";
			continue;
		}

		if (Combo_->findText (sii.Page_) == -1)
		{
			QWidget *w = new QWidget;
			QVBoxLayout *lay = new QVBoxLayout;
			w->setLayout (lay);

			Combo_->addItem (sii.Page_);
			Pages_->addWidget (w);
		}

		int position = Combo_->findText (sii.Page_);
		QVBoxLayout *layToAdd;

		if (sii.Group_.isEmpty ())
			layToAdd = qobject_cast<QVBoxLayout*> (Pages_->widget (position)->layout ());
		else
		{
			QGroupBox *groupBox = 0;
			for (int i = 0; i < GroupBoxes_.size (); ++i)
				if (GroupBoxes_ [i]->objectName () == sii.Group_)
				{
					groupBox = GroupBoxes_ [i];
					break;
				}
			if (!groupBox)
			{
				groupBox = new QGroupBox (sii.Group_);
				groupBox->setObjectName (sii.Group_);
				groupBox->setLayout (new QVBoxLayout);
				GroupBoxes_.append (groupBox);
				QVBoxLayout *wlay = qobject_cast<QVBoxLayout*> (Pages_->widget (position)->layout ());
				wlay->addWidget (groupBox);
			}

			layToAdd = qobject_cast<QVBoxLayout*> (groupBox->layout ());
		}

		if (TypeHandler_->MakeLabel (object->property (propName.toAscii ().data ()), sii))
		{
			QHBoxLayout *tmplay = new QHBoxLayout;
			tmplay->addWidget (new QLabel (sii.Label_, this));
			tmplay->addWidget (representator);
			layToAdd->addLayout (tmplay);
		}
		else
			layToAdd->addWidget (representator);
	}

	for (int i = 0; i < Pages_->count (); ++i)
	{
		QVBoxLayout *lay = qobject_cast<QVBoxLayout*> (Pages_->widget (i)->layout ());
		lay->insertStretch (-1, 1);
	}
}

void SettingsDialog::accept ()
{
	TypeHandler_->UpdateSettings ();
	QDialog::accept ();
}

void SettingsDialog::doAdjustSize ()
{
	Pages_->currentWidget ()->adjustSize ();
	adjustSize ();
}

