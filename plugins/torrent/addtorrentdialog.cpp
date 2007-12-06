#include <QHeaderView>
#include <QFileDialog>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "addtorrentdialog.h"
#include "addtorrentwidgetitem.h"

AddTorrentDialog::AddTorrentDialog (const QString& lod, const QString& lsd, QWidget *parent)
: QDialog (parent)
, LastOpenDir_ (lod)
, LastSaveDir_ (lsd)
{
	setupUi (this);

	if (lod.isEmpty ())
		LastOpenDir_ = QDir::currentPath ();
	if (lsd.isEmpty ())
		LastSaveDir_ = QDir::currentPath ();

	QStringList tfl;
	tfl << tr ("?") << tr ("File") << tr ("Size");
	TorrentFiles_->setHeaderLabels (tfl);
	TorrentFiles_->header ()->setStretchLastSection (true);
	TorrentFiles_->header ()->setHighlightSections (false);
	TorrentFiles_->header ()->setDefaultAlignment (Qt::AlignLeft);

	connect (TorrentBrowseButton_, SIGNAL (clicked ()), this, SLOT (pickTorrentFile ()));
	connect (DestBrowseButton_, SIGNAL (clicked ()), this, SLOT (selectDestination ()));
	connect (TorrentFile_, SIGNAL (textChanged (QString)), this, SIGNAL (fileChanged (QString)));
	connect (TorrentFiles_, SIGNAL (itemClicked (QTreeWidgetItem*, int)), this, SLOT (toggleItem (QTreeWidgetItem*, int)));
}

void AddTorrentDialog::SetFiles (QList<QPair<QString, int> > files)
{
	for (int i = 0; i < files.size (); ++i)
	{
		AddTorrentWidgetItem *twi = new AddTorrentWidgetItem;
		twi->setData (0, Qt::CheckStateRole, Qt::Checked);
		twi->setText (1, files [i].first);
		twi->setText (2, Proxy::Instance ()->MakePrettySize (files [i].second));

		TorrentFiles_->addTopLevelItem (twi);
	}

	for (int i = 0; i < TorrentFiles_->columnCount (); ++i)
		TorrentFiles_->resizeColumnToContents (i);
}

void AddTorrentDialog::Reinit (const QString& lod, const QString& lsd)
{
	TorrentFiles_->clear ();
	TorrentFile_->clear ();
	LastOpenDir_ = lod;
	LastSaveDir_ = lsd;
}

QStringList AddTorrentDialog::GetFiles2Download () const
{
	QStringList result;
	for (int i = 0; i < TorrentFiles_->topLevelItemCount (); ++i)
		if (TorrentFiles_->topLevelItem (i)->data (0, Qt::CheckStateRole).toInt () == Qt::Checked)
			result << TorrentFiles_->topLevelItem (i)->text (1);

	return result;
}

QString AddTorrentDialog::GetFilename () const
{
	return TorrentFile_->text ();
}

QString AddTorrentDialog::GetDestDir () const
{
	return DestPath_->text ();
}

void AddTorrentDialog::pickTorrentFile ()
{
	QString filename = QFileDialog::getOpenFileName (this, tr ("Select torrent file"), LastOpenDir_, tr ("Torrents (*.torrent)"));
	if (filename.isEmpty ())
		return;

	TorrentFile_->setText (filename);
}

void AddTorrentDialog::selectDestination ()
{
	QString destDir = QFileDialog::getExistingDirectory (this, tr ("Select save directory"), LastSaveDir_); 
	if (destDir.isEmpty ())
		return;

	DestPath_->setText (destDir);
}

void AddTorrentDialog::toggleItem (QTreeWidgetItem *item, int column)
{
	qDebug () << Q_FUNC_INFO;
	if (column == 0)
		return;
	item->setData (0, Qt::CheckStateRole, (item->data (0, Qt::CheckStateRole).toInt () == Qt::Checked ? Qt::Unchecked : Qt::Checked));
}

