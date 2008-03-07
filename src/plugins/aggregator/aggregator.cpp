#include <QMessageBox>
#include <QtDebug>
#include "aggregator.h"
#include "core.h"
#include "addfeed.h"

void Aggregator::Init ()
{
    Ui_.setupUi (this);
    IsShown_ = false;
    Plugins_->addAction (Ui_.ActionAddFeed_);
    connect (&Core::Instance (), SIGNAL (error (const QString&)), this, SLOT (showError (const QString&)));
    Ui_.Items_->setModel (&Core::Instance ());
}

void Aggregator::Release ()
{
}

QString Aggregator::GetName () const
{
    return "Aggregator";
}

QString Aggregator::GetInfo () const
{
    return tr ("RSS 2.0 feed reader.");
}

QString Aggregator::GetStatusbarMessage () const
{
    return QString ();
}

IInfo& Aggregator::SetID (long unsigned int id)
{
    ID_ = id;
    return *this;
}

unsigned long int Aggregator::GetID () const
{
    return ID_;
}

QStringList Aggregator::Provides () const
{
    return QStringList ("rss");
}

QStringList Aggregator::Needs () const
{
    return QStringList ("http");
}

QStringList Aggregator::Uses () const
{
    return QStringList ();
}

void Aggregator::SetProvider (QObject* object, const QString& feature)
{
    Core::Instance ().SetProvider (object, feature);
}

void Aggregator::PushMainWindowExternals (const MainWindowExternals& externals)
{
    Plugins_ = externals.RootMenu_->addMenu ("&Aggregator");
}

QIcon Aggregator::GetIcon () const
{
    return windowIcon ();
}

void Aggregator::SetParent (QWidget *parent)
{
    setParent (parent);
}

void Aggregator::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void Aggregator::ShowBalloonTip ()
{
}

void Aggregator::closeEvent (QCloseEvent*)
{
    IsShown_ = false;
}

void Aggregator::showError (const QString& msg)
{
    qDebug () << Q_FUNC_INFO << msg;
    QMessageBox::warning (this, tr ("Error"), msg);
}

void Aggregator::on_ActionAddFeed__triggered ()
{
    AddFeed af;
    if (af.exec () == QDialog::Accepted)
        Core::Instance ().AddFeed (af.GetURL ());
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, Aggregator);

