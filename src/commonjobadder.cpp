#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

LeechCraft::CommonJobAdder::CommonJobAdder (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
	What_->setText (XmlSettingsManager::Instance ()->Property ("LastWhatFolder",
				QDir::homePath ()).toString ());
	Where_->setText (XmlSettingsManager::Instance ()->Property ("LastWhereFolder",
				QDir::homePath ()).toString ());
}

LeechCraft::CommonJobAdder::~CommonJobAdder ()
{
}

QString LeechCraft::CommonJobAdder::GetString () const
{
	return What_->text ();
}

QString LeechCraft::CommonJobAdder::GetWhere () const
{
	return Where_->text ();
}

void LeechCraft::CommonJobAdder::on_Browse__released ()
{
	QString name = QFileDialog::getOpenFileName (this,
			tr ("Select file"),
			XmlSettingsManager::Instance ()->Property ("LastWhatFolder",
				QDir::homePath ()).toString ());
	if (name.isEmpty ())
		return;

	What_->setText (name);
	XmlSettingsManager::Instance ()->setProperty ("LastWhatFolder", name);
}

void LeechCraft::CommonJobAdder::on_BrowseWhere__released ()
{
	QString name = QFileDialog::getExistingDirectory (this,
			tr ("Select file"),
			XmlSettingsManager::Instance ()->Property ("LastWhereFolder",
				QDir::homePath ()).toString ());
	if (name.isEmpty ())
		return;

	Where_->setText (name);
	XmlSettingsManager::Instance ()->setProperty ("LastWhereFolder", name);
}

void LeechCraft::CommonJobAdder::on_Paste__released ()
{
	QString text = QApplication::clipboard ()->text ();
	What_->setText (text.split ('\n') [0]);
}

