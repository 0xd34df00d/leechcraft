#include <QHeaderView>
#include <QFileDialog>
#include <plugininterface/proxy.h>
#include <boost/date_time.hpp>
#include "addtorrent.h"
#include "xmlsettingsmanager.h"
#include "core.h"

AddTorrent::AddTorrent (QWidget *parent)
: QDialog (parent)
{
   setupUi (this);
   FileWidget_->header ()->setStretchLastSection (true);
   OK_->setEnabled (false);
   connect (this, SIGNAL (on_TorrentFile__textChanged ()), this, SLOT (setOkEnabled ()));
   connect (this, SIGNAL (on_Destination__textChanged ()), this, SLOT (setOkEnabled ()));

   QString dir = XmlSettingsManager::Instance ()->property ("LastSaveDirectory").toString ();
   Destination_->setText (dir);
}

void AddTorrent::Reinit ()
{
   while (FileWidget_->topLevelItemCount ())
      delete FileWidget_->takeTopLevelItem (0);
   TorrentFile_->setText ("");
   TrackerURL_->setText (tr ("<unknown>"));
   Size_->setText (tr ("<unknown>"));
   Creator_->setText (tr ("<unknown>"));
   Comment_->setText (tr ("<unknown>"));
   Date_->setText (tr ("<unknown>"));
}

void AddTorrent::SetFilename (const QString& filename)
{
   if (filename.isEmpty ())
      return;

   Reinit ();

   XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory", QFileInfo (filename).absolutePath ());
   TorrentFile_->setText (filename);

   ParseBrowsed ();
}

QString AddTorrent::GetFilename () const
{
   return TorrentFile_->text ();
}

QString AddTorrent::GetSavePath () const
{
   return Destination_->text ();
}

QVector<bool> AddTorrent::GetSelectedFiles () const
{
   QVector<bool> result;
   for (int i = 0; i < FileWidget_->topLevelItemCount (); ++i)
      result.append (FileWidget_->topLevelItem (i)->checkState (0) == Qt::Checked);
   return result;
}

void AddTorrent::setOkEnabled ()
{
   OK_->setEnabled (QFileInfo (TorrentFile_->text ()).isReadable () && QFileInfo (Destination_->text ()).exists ());
}

void AddTorrent::on_TorrentBrowse__released ()
{
   QString filename = QFileDialog::getOpenFileName (this, tr ("Select torrent file"), XmlSettingsManager::Instance ()->property ("LastTorrentDirectory").toString (), tr ("Torrents (*.torrent);;All files (*.*)"));
   if (filename.isEmpty ())
      return;

   Reinit ();

   XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory", QFileInfo (filename).absolutePath ());
   TorrentFile_->setText (filename);

   ParseBrowsed ();
}

void AddTorrent::on_DestinationBrowse__released ()
{
   QString dir = QFileDialog::getExistingDirectory (this, tr ("Select save directory"), Destination_->text (), 0);
   if (dir.isEmpty ())
      return;

   XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
   Destination_->setText (dir);
}

void AddTorrent::ParseBrowsed ()
{
   QString filename = TorrentFile_->text ();
   libtorrent::torrent_info info = Core::Instance ()->GetTorrentInfo (filename);
   if (!info.is_valid ())
      return;
   TrackerURL_->setText (QString::fromStdString (info.trackers ().at (0).url));
   Size_->setText (Proxy::Instance ()->MakePrettySize (info.total_size ()));
   QString creator = QString::fromStdString (info.creator ()),
         comment = QString::fromStdString (info.comment ());
   QString date = QString::fromStdString (boost::posix_time::to_simple_string (info.creation_date ().get ()));
   if (!creator.isEmpty () && !creator.isNull ())
      Creator_->setText (creator);
   if (!comment.isEmpty () && !comment.isNull ())
      Comment_->setText (comment);
   if (!date.isEmpty () && !date.isNull ())
      Date_->setText (date);
   for (libtorrent::torrent_info::file_iterator i = info.begin_files (); i != info.end_files (); ++i)
   {
      QTreeWidgetItem *item = new QTreeWidgetItem (FileWidget_);
      item->setCheckState (0, Qt::Checked);
      item->setText (0, Proxy::Instance ()->MakePrettySize (i->size));
      item->setText (1, QString::fromStdString (i->path.string ()));
   }
}

