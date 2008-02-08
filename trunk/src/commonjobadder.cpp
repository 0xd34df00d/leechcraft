#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"

CommonJobAdder::CommonJobAdder (QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
}

QString CommonJobAdder::GetString () const
{
    return What_->text ();
}

void CommonJobAdder::on_Browse__released ()
{
    QString name = QFileDialog::getOpenFileName (this, tr ("Select file"), XmlSettingsManager::Instance ()->Property ("LastCommonFolder", QDir::homePath ()).toString ());
    if (name.isEmpty ())
        return;

    What_->setText (name);
    XmlSettingsManager::Instance ()->setProperty ("LastCommonFolder", name);
}

void CommonJobAdder::on_Paste__released ()
{
    QString text = QApplication::clipboard ()->text ();
    What_->setText (text.split ('\n') [0]);
}

