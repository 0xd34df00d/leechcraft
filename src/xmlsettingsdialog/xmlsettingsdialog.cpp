#include <QFile>
#include <QtGui/QtGui>
#include <QtXml/QtXml>
#include <QtDebug>
#include "xmlsettingsdialog.h"
#include "rangewidget.h"
#include "filepicker.h"
#include "radiogroup.h"

XmlSettingsDialog::XmlSettingsDialog (QWidget *parent)
: QDialog (parent)
{
    Pages_ = new QStackedWidget;
    Sections_ = new QListWidget;
    Sections_->setMinimumWidth (100);
    Sections_->setMaximumWidth (150);

    connect (Sections_, SIGNAL (currentRowChanged (int)), Pages_, SLOT (setCurrentIndex (int)));

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

    DefaultLang_ = "en";
}

void XmlSettingsDialog::RegisterObject (QObject* obj, const QString& filename)
{
    WorkingObject_ = obj;
    QFile file (filename);
    if (!file.open (QIODevice::ReadOnly))
    {
        qWarning () << "cannot open file";
        return;
    }
    QByteArray data = file.readAll ();
    file.close ();

    QDomDocument document;
    if (!document.setContent (data))
    {
        qWarning () << "Could not parse file";
        return;
    }
    QDomElement root = document.documentElement ();
    if (root.tagName () != "settings")
    {
        qWarning () << "Bad settings file";
        return;
    }

    QDomElement declaration = root.firstChildElement ("declare");
    while (!declaration.isNull ())
    {
        HandleDeclaration (declaration);
        declaration = declaration.nextSiblingElement ("declare");
    }

    QDomElement pageChild = root.firstChildElement ("page");
    while (!pageChild.isNull ())
    {
        ParsePage (pageChild);
        pageChild = pageChild.nextSiblingElement ("page");
        adjustSize ();
    }
}

void XmlSettingsDialog::HandleDeclaration (const QDomElement& decl)
{
    if (decl.hasAttribute ("defaultlang"))
        DefaultLang_ = decl.attribute ("defaultlang");
}

void XmlSettingsDialog::ParsePage (const QDomElement& page)
{
    QString sectionTitle = GetLabel (page);
    Sections_->addItem (sectionTitle);

    QWidget *baseWidget = new QWidget;
    Pages_->addWidget (baseWidget);
    QGridLayout *lay = new QGridLayout;
    baseWidget->setLayout (lay);

    ParseEntity (page, baseWidget);
    lay->setRowStretch (lay->rowCount (), 1);
}

void XmlSettingsDialog::ParseEntity (const QDomElement& entity, QWidget *baseWidget)
{
    QDomElement item = entity.firstChildElement ("item");
    while (!item.isNull ())
    {
        ParseItem (item, baseWidget);
        item = item.nextSiblingElement ("item");
    }

    QDomElement gbox = entity.firstChildElement ("groupbox");
    while (!gbox.isNull ())
    {
        QGroupBox *box = new QGroupBox (GetLabel (gbox));
        box->setLayout (new QGridLayout);
        ParseEntity (gbox, box);
        
        QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
        int row = lay->rowCount ();
        lay->addWidget (box, row, 0, 1, 2);

        gbox = gbox.nextSiblingElement ("groupbox");
    }

    QDomElement tab = entity.firstChildElement ("tab");
    if (!tab.isNull ())
    {
        QTabWidget *tabs = new QTabWidget;
        QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
        int row = lay->rowCount ();
        lay->addWidget (tabs, row, 0, 1, 2);
        while (!tab.isNull ())
        {
            QWidget *page = new QWidget;
            QGridLayout *widgetLay = new QGridLayout;
            page->setLayout (widgetLay);
            tabs->addTab (page, GetLabel (tab));
            ParseEntity (tab, page);
            tab = tab.nextSiblingElement ("tab");
            widgetLay->setRowStretch (widgetLay->rowCount (), 1);
        }
    }
}

void XmlSettingsDialog::ParseItem (const QDomElement& item, QWidget *baseWidget)
{
    QString type = item.attribute ("type");

    QGridLayout *lay = qobject_cast<QGridLayout*> (baseWidget->layout ());
    int row = lay->rowCount ();

    QString property = item.attribute ("property");
    QVariant value = WorkingObject_->property (property.toLatin1 ().constData ());
    if (!value.isValid () || value.isNull () && item.hasAttribute ("default"))
    {
        value = item.attribute ("default");
        WorkingObject_->setProperty (property.toLatin1 ().constData (), value);
    }

    if (type.isEmpty () || type.isNull ())
        return;
    else if (type == "lineedit")
        DoLineedit (item, lay, value);
    else if (type == "checkbox")
        DoCheckbox (item, lay, value);
    else if (type == "spinbox")
        DoSpinbox (item, lay, value);
    else if (type == "groupbox" && item.attribute ("checkable") == "true")
        DoGroupbox (item, lay, value);
    else if (type == "spinboxrange")
        DoSpinboxRange (item, lay, value);
    else if (type == "path")
        DoPath (item, lay, value);
    else if (type == "radio")
        DoRadio (item, lay, value);
    else if (type == "combobox")
        DoCombobox (item, lay, value);
    else
        qWarning () << Q_FUNC_INFO << "unhandled type" << type;

    WorkingObject_->setProperty (property.toLatin1 ().constData (), value);
}

QString XmlSettingsDialog::GetLabel (const QDomElement& item) const
{
    QString locale = QLocale::system ().name ().toLower ();
    if (locale == "c")
        locale = "en";

    locale = locale.left (2);

    QString result;
    QDomElement label = item.firstChildElement ("label");
    while (!label.isNull ())
    {
        if (label.attribute ("lang").toLower () == locale)
        {
            result = label.attribute ("value");
            break;
        }
        label = label.nextSiblingElement ("label");
    }
    if (result.isEmpty ())
    {
        label = item.firstChildElement ("label");
        while (!label.isNull ())
        {
            if (label.attribute ("lang").toLower () == DefaultLang_)
            {
                result = label.attribute ("value");
                break;
            }
            label = label.nextSiblingElement ("label");
        }
    }
    return result;
}

XmlSettingsDialog::LangElements XmlSettingsDialog::GetLangElements (const QDomElement& parent) const
{
    QString locale = QLocale::system ().name ().toLower ();
    if (locale == "c")
        locale = "en";

    locale = locale.left (2);
    LangElements returning;
    returning.Valid_ = false;

    bool found = false;

    QDomElement result = parent.firstChildElement ("lang");
    while (!result.isNull ())
    {
        if (result.attribute ("value").toLower () == locale)
        {
            found = true;
            break;
        }
        result = result.nextSiblingElement ("lang");
    }
    if (!found)
    {
        result = parent.firstChildElement ("lang");
        while (!result.isNull ())
        {
            if (result.attribute ("value").toLower () == DefaultLang_)
            {
                found = true;
                break;
            }
            result = result.nextSiblingElement ("lang");
        }
    }
    if (!found)
    {
        result = parent.firstChildElement ("lang");
        while (!result.isNull ())
        {
            if (result.attribute ("value").toLower () == "en" || !result.hasAttribute ("value"))
            {
                found = true;
                break;
            }
            result = result.nextSiblingElement ("lang");
        }
    }
    if (result.isNull ())
        return returning;

    returning.Valid_ = true;

    QDomElement label = result.firstChildElement ("label");
    if (!label.isNull () && label.hasAttribute ("value"))
    {
        returning.Label_.first = true;
        returning.Label_.second = label.attribute ("value");
    }
    else
        returning.Label_.first = false;

    QDomElement suffix = result.firstChildElement ("suffix");
    if (!suffix.isNull () && suffix.hasAttribute ("value"))
    {
        returning.Suffix_.first = true;
        returning.Suffix_.second = suffix.attribute ("value");
    }
    else
        returning.Suffix_.first = false;

    return returning;
}

void XmlSettingsDialog::DoLineedit (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    int row = lay->rowCount ();
    QLabel *label = new QLabel (GetLabel (item));

    QLineEdit *edit = new QLineEdit (value.toString ());
    edit->setObjectName (item.attribute ("property"));
    edit->setMinimumWidth (QApplication::fontMetrics ().width ("thisismaybeadefaultsettingstring,dontyouthinkso?"));
    if (item.hasAttribute ("password"))
        edit->setEchoMode (QLineEdit::Password);
    connect (edit, SIGNAL (textChanged (const QString&)), this, SLOT (updatePreferences ()));

    lay->addWidget (label, row, 0);
    lay->addWidget (edit, row, 1);
}

void XmlSettingsDialog::DoCheckbox (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    QCheckBox *box = new QCheckBox (GetLabel (item));
    box->setObjectName (item.attribute ("property"));
    if (!value.isValid () || value.isNull ())
        value = (item.attribute ("state") == "on");
    box->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
    connect (box, SIGNAL (stateChanged (int)), this, SLOT (updatePreferences ()));

    lay->addWidget (box, lay->rowCount (), 0, 1, 2);
}

void XmlSettingsDialog::DoSpinbox (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    int row = lay->rowCount ();
    QLabel *label = new QLabel (GetLabel (item));
    QSpinBox *box = new QSpinBox;
    box->setObjectName (item.attribute ("property"));
    if (item.hasAttribute ("minimum"))
        box->setMinimum (item.attribute ("minimum").toInt ());
    if (item.hasAttribute ("maximum"))
        box->setMaximum (item.attribute ("maximum").toInt ());
    if (item.hasAttribute ("step"))
        box->setSingleStep (item.attribute ("step").toInt ());
    if (item.hasAttribute ("suffix"))
        box->setSuffix (item.attribute ("suffix"));
    LangElements langs = GetLangElements (item);
    qDebug () << Q_FUNC_INFO << langs.Valid_;
    if (langs.Valid_)
    {
        if (langs.Label_.first)
            label->setText (langs.Label_.second);
        if (langs.Suffix_.first)
            box->setSuffix (langs.Suffix_.second);
    }
    box->setValue (value.toInt ());
    connect (box, SIGNAL (valueChanged (int)), this, SLOT (updatePreferences ()));
    
    lay->addWidget (label, row, 0);
    lay->addWidget (box, row, 1);
}

void XmlSettingsDialog::DoGroupbox (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    QGroupBox *box = new QGroupBox (GetLabel (item));
    box->setObjectName (item.attribute ("property"));
    box->setLayout (new QGridLayout);
    box->setCheckable (true);
    if (!value.isValid () || value.isNull ())
        value = (item.attribute ("state") == "on");
    box->setChecked (value.toBool ());
    connect (box, SIGNAL (toggled (bool)), this, SLOT (updatePreferences ()));
    ParseEntity (item, box);
    
    lay->addWidget (box, lay->rowCount (), 0, 1, 2);
}

void XmlSettingsDialog::DoSpinboxRange (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    if (!value.isValid () || value.isNull () || !value.canConvert<QList<QVariant> > ())
    {
        QStringList parts = item.attribute ("default").split (":");
        QList<QVariant> result;
        if (parts.size () != 2)
        {
            qWarning () << "spinboxrange parse error, wrong default value";
            return;
        }
        result << parts.at (0).toInt () << parts.at (1).toInt ();
        value = result;
    }

    QLabel *label = new QLabel (GetLabel (item));
    RangeWidget *widget = new RangeWidget ();
    widget->setObjectName (item.attribute ("property"));
    widget->SetMinimum (item.attribute ("minimum").toInt ());
    widget->SetMaximum (item.attribute ("maximum").toInt ());
    widget->SetRange (value);
    connect (widget, SIGNAL (changed ()), this, SLOT (updatePreferences ()));

    int row = lay->rowCount ();
    lay->addWidget (label, row, 0);
    lay->addWidget (widget, row, 1);
}

void XmlSettingsDialog::DoPath (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    if (value.isNull () || value.toString ().isEmpty ())
        if (item.hasAttribute ("defaultHomePath") && item.attribute ("defaultHomePath") == "true")
            value = QDir::homePath ();
    QLabel *label = new QLabel (GetLabel (item));
    FilePicker *picker = new FilePicker (this);
    picker->SetText (value.toString ());
    picker->setObjectName (item.attribute ("property"));
    connect (picker, SIGNAL (textChanged (const QString&)), this, SLOT (updatePreferences ()));

    int row = lay->rowCount ();
    lay->addWidget (label, row, 0);
    lay->addWidget (picker, row, 1);
}

void XmlSettingsDialog::DoRadio (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    RadioGroup *group = new RadioGroup (this);
    group->setObjectName (item.attribute ("property"));

    QDomElement option = item.firstChildElement ("option");
    while (!option.isNull ())
    {
        QRadioButton *button = new QRadioButton (GetLabel (option));
        button->setObjectName (option.attribute ("name"));
        group->AddButton (button, option.hasAttribute ("default") && option.attribute ("default") == "true");
        if (option.attribute ("default") == "true")
            value = option.attribute ("name");
        option = option.nextSiblingElement ("option");
    }
    connect (group, SIGNAL (valueChanged ()), this, SLOT (updatePreferences ()));

    QGroupBox *box = new QGroupBox (GetLabel (item));
    QVBoxLayout *layout = new QVBoxLayout;
    box->setLayout (layout);
    layout->addWidget (group);
    lay->addWidget (box, lay->rowCount (), 0, 1, 2);
}

void XmlSettingsDialog::DoCombobox (const QDomElement& item, QGridLayout *lay, QVariant& value)
{
    QComboBox *box = new QComboBox (this);
    box->setObjectName (item.attribute ("property"));

    QDomElement option = item.firstChildElement ("option");
    while (!option.isNull ())
    {
        box->addItem (GetLabel (option), option.attribute ("name"));
        if (option.attribute ("default") == "true")
        {
            box->setCurrentIndex (box->count () - 1);
            value = option.attribute ("name");
        }
        option = option.nextSiblingElement ("option");
    }
    connect (box, SIGNAL (currentIndexChanged (int)), this, SLOT (updatePreferences ()));

    QLabel *label = new QLabel (GetLabel (item));
    int row = lay->rowCount ();
    lay->addWidget (label, row, 0);
    lay->addWidget (box, row, 1);
}

void XmlSettingsDialog::updatePreferences ()
{
    QString propertyName = sender ()->objectName ();
    if (propertyName.isEmpty ())
    {
        qWarning () << Q_FUNC_INFO << "property name is empty for object" << sender ();
        return;
    }
    QVariant value;

    QLineEdit *edit = qobject_cast<QLineEdit*> (sender ());
    QCheckBox *checkbox = qobject_cast<QCheckBox*> (sender ());
    QSpinBox *spinbox = qobject_cast<QSpinBox*> (sender ());
    QGroupBox *groupbox = qobject_cast<QGroupBox*> (sender ());
    RangeWidget *rangeWidget = qobject_cast<RangeWidget*> (sender ());
    FilePicker *picker = qobject_cast<FilePicker*> (sender ());
    RadioGroup *radiogroup = qobject_cast<RadioGroup*> (sender ());
    QComboBox *combobox = qobject_cast<QComboBox*> (sender ());
    if (edit)
        value = edit->text ();
    else if (checkbox)
        value = checkbox->checkState ();
    else if (spinbox)
        value = spinbox->value ();
    else if (groupbox)
        value = groupbox->isChecked ();
    else if (rangeWidget)
        value = rangeWidget->GetRange ();
    else if (picker)
        value = picker->GetText ();
    else if (radiogroup)
        value = radiogroup->GetValue ();
    else if (combobox)
        value = combobox->itemData (combobox->currentIndex ());
    else
    {
        qWarning () << Q_FUNC_INFO << "unhandled sender" << sender ();
        return;
    }

    Prop2NewValue_ [propertyName] = value;
}

void XmlSettingsDialog::accept ()
{
    for (Property2Value_t::const_iterator i = Prop2NewValue_.begin (); i != Prop2NewValue_.end (); ++i)
        WorkingObject_->setProperty (i.key ().toLatin1 ().constData (), i.value ());

    QDialog::accept ();
}

