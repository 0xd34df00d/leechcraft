#include <QVariant>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
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
   Pages_ = new QStackedWidget ();
   Sections_ = new QListWidget ();
   Sections_->setMaximumWidth (150);
   Sections_->setMinimumWidth (100);
   Sections_->setIconSize (QSize (32, 32));
//   Sections_->setViewMode (QListView::IconMode);

   connect (Sections_, SIGNAL (currentRowChanged (int)), Pages_, SLOT (setCurrentIndex (int)));
   connect (Sections_, SIGNAL (currentRowChanged (int)), this, SLOT (doAdjustSize ()));

   OK_ = new QPushButton (tr ("OK"));
   Cancel_ = new QPushButton (tr ("Cancel"));

   QHBoxLayout *buttons = new QHBoxLayout;
   buttons->addStretch (1);
   buttons->addWidget (OK_);
   buttons->addWidget (Cancel_);
   connect (OK_, SIGNAL (released ()), this, SLOT (accept ()));
   connect (Cancel_, SIGNAL (released ()), this, SLOT (reject ()));

   QVBoxLayout *rightLay = new QVBoxLayout;
   QHBoxLayout *mainLay = new QHBoxLayout (this);
   mainLay->addWidget (Sections_);
   rightLay->addWidget (Pages_);
   rightLay->addStretch (1);
   mainLay->addLayout (rightLay);
   rightLay->addLayout (buttons);
   setLayout (mainLay);

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

   Objects_.append (const_cast<QObject*> (object));

   const QMetaObject *mo = object->metaObject ();

   CurrentProps_.clear ();
   for (int i = mo->propertyOffset (); i < mo->propertyCount (); ++i)
      CurrentProps_ << mo->property (i);

   for (int i = 0; i < CurrentProps_.size (); ++i)
   {
      QMetaProperty currentProp = CurrentProps_.at (i);
      QString propName = currentProp.name ();

      SettingsItemInfo sii = castedObject->GetInfoFor (propName);

      QWidget *representator = (*TypeHandler_) (object->property (propName.toAscii ().data ()), sii, propName, object);
      if (!representator)
      {
         qWarning () << "Creating representator widget failed for type" << object->property (propName.toAscii ().data ()) << ", continuing...";
         continue;
      }

      if (Sections_->findItems (sii.Page_, Qt::MatchExactly).isEmpty ())
      {
         QWidget *w = new QGroupBox (sii.Page_);
         QVBoxLayout *lay = new QVBoxLayout;
         w->setLayout (lay);

         QListWidgetItem *item = new QListWidgetItem;
         item->setText (sii.Page_);
         Sections_->addItem (item);
         Pages_->addWidget (w);
      }

      QListWidgetItem *item = Sections_->findItems (sii.Page_, Qt::MatchExactly).first ();
      if (!sii.PageIcon_.isNull ())
         item->setIcon (sii.PageIcon_);

      int position = Sections_->row (item);
      QVBoxLayout *layToAdd;

      if (PropertyToParentWidget_.contains (propName))
         layToAdd = qobject_cast<QVBoxLayout*> (PropertyToParentWidget_ [propName]->layout ());
      else if (sii.Group_.isEmpty ())
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

QList<QMetaProperty>& SettingsDialog::AccessProps ()
{
   return CurrentProps_;
}

void SettingsDialog::SetParentWidgetForProperty (const QString& prop, QWidget *widget)
{
   PropertyToParentWidget_ [prop] = widget;
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

