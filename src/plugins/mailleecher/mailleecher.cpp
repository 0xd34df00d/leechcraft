#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include "mailleecher.h"
#include "lcmlparser.h"
#include "core.h"

void MailLeecher::Init ()
{
    QTranslator *transl = new QTranslator (this);
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_mailleecher_") + localeName);
    qApp->installTranslator (transl);

    IsShown_ = false;
    IsLeeching_ = false;
    Ui_.setupUi (this);
    setActionsEnabled ();
    connect (Core::Instance (), SIGNAL (error (const QString&)), this, SLOT (showError (const QString&)));
    connect (Core::Instance (), SIGNAL (log (const QString&)), this, SLOT (doLog (const QString&)));
    connect (Core::Instance (), SIGNAL (dataProgress (int)), Ui_.DataBar_, SLOT (setValue (int)));
    connect (Core::Instance (), SIGNAL (mailProgress (int)), Ui_.MessagesBar_, SLOT (setValue (int)));
    connect (Core::Instance (), SIGNAL (totalData (int)), Ui_.DataBar_, SLOT (setMaximum (int)));
    connect (Core::Instance (), SIGNAL (totalMail (int)), Ui_.MessagesBar_, SLOT (setMaximum (int)));
}

void MailLeecher::Release ()
{
}

QString MailLeecher::GetName () const
{
    return "MailLeecher";
}

QString MailLeecher::GetInfo () const
{
    return tr ("Allows you to back up your POP3 mailbox");
}

QString MailLeecher::GetStatusbarMessage () const
{
    return tr ("NI");
}

IInfo& MailLeecher::SetID (ulong id)
{
    ID_ = id;
}

ulong MailLeecher::GetID () const
{
    return ID_;
}

QStringList MailLeecher::Provides () const
{
    return QStringList ("pop3");
}

QStringList MailLeecher::Needs () const
{
    return QStringList ();
}

QStringList MailLeecher::Uses () const
{
    return QStringList ();
}

void MailLeecher::SetProvider (QObject*, const QString&)
{
}

void MailLeecher::PushMainWindowExternals (const MainWindowExternals&)
{
}

QIcon MailLeecher::GetIcon () const
{
    return windowIcon ();
}

void MailLeecher::SetParent (QWidget *parent)
{
    setParent (parent);
}

void MailLeecher::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void MailLeecher::ShowBalloonTip ()
{
}

void MailLeecher::closeEvent (QEvent*)
{
    IsShown_ = false;
}

void MailLeecher::handleHidePlugins ()
{
    IsShown_ = false;
    hide ();
}

void MailLeecher::on_ServerAddress__textChanged ()
{
    setActionsEnabled ();
}

void MailLeecher::on_OutputDirectory__textChanged ()
{
    setActionsEnabled ();
}

void MailLeecher::on_Login__textChanged ()
{
    setActionsEnabled ();
}

void MailLeecher::on_OutputBrowse__released ()
{
    QString dir = QFileDialog::getExistingDirectory (this, tr ("Select destination directory"), QDir::homePath (), 0);
    if (dir.isEmpty ())
        return;
    Ui_.OutputDirectory_->setText (dir);
}

void MailLeecher::on_Leech__released ()
{
    Core::Instance ()->StartDownload (Ui_.ServerAddress_->text (), Ui_.ServerPort_->value (),
            Ui_.Login_->text (), Ui_.Password_->text (), Ui_.OutputDirectory_->text ());
}

void MailLeecher::setActionsEnabled ()
{
    if (!Ui_.ServerAddress_->text ().isEmpty () && !Ui_.OutputDirectory_->text ().isEmpty () && !Ui_.Login_->text ().isEmpty ())
        Ui_.Leech_->setDisabled (false);
    else
        Ui_.Leech_->setDisabled (true);

    Ui_.Cancel_->setDisabled (!IsLeeching_);
}

void MailLeecher::showError (const QString& msg)
{
    QMessageBox::warning (this, tr ("Warning"), msg);
}

void MailLeecher::doLog (const QString&)
{
}

Q_EXPORT_PLUGIN2 (leechcraft_mailleecher, MailLeecher)

