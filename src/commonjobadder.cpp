#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"

CommonJobAdder::CommonJobAdder (QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
	Where_->setText (Main::XmlSettingsManager::Instance ()->Property ("LastCommonFolder",
				QDir::homePath ()).toString ());
}

QString CommonJobAdder::GetString () const
{
    return What_->text ();
}

QString CommonJobAdder::GetWhere () const
{
	return Where_->text ();
}

void CommonJobAdder::on_Browse__released ()
{
    QString name = QFileDialog::getOpenFileName (this,
			tr ("Select file"),
			Main::XmlSettingsManager::Instance ()->Property ("LastCommonFolder",
				QDir::homePath ()).toString ());
    if (name.isEmpty ())
        return;

    What_->setText (name);
    Main::XmlSettingsManager::Instance ()->setProperty ("LastCommonFolder", name);
}

void CommonJobAdder::on_BrowseWhere__released ()
{
    QString name = QFileDialog::getOpenFileName (this,
			tr ("Select file"),
			Main::XmlSettingsManager::Instance ()->Property ("LastWhereFolder",
				QDir::homePath ()).toString ());
    if (name.isEmpty ())
        return;

    Where_->setText (name);
    Main::XmlSettingsManager::Instance ()->setProperty ("LastWhereFolder", name);
}

void CommonJobAdder::on_Paste__released ()
{
    QString text = QApplication::clipboard ()->text ();
    What_->setText (text.split ('\n') [0]);
}

