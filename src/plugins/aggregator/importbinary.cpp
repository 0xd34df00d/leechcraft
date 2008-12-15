#include "importbinary.h"
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QtDebug>

ImportBinary::ImportBinary (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
}

ImportBinary::~ImportBinary ()
{
}

QString ImportBinary::GetFilename () const
{
	return Ui_.File_->text ();
}

QString ImportBinary::GetTags () const
{
	return Ui_.AdditionalTags_->text ().trimmed ();
}

namespace
{
	struct ParentFinder
	{
		const QString& URL_;

		ParentFinder (const QString& url)
		: URL_ (url)
		{
		}

		bool operator() (const Feed_ptr& feed) const
		{
			return URL_ == feed->URL_;
		}
	};
};

feeds_container_t ImportBinary::GetSelectedFeeds () const
{
	feeds_container_t result;

	for (int i = 0, end = Ui_.FeedsToImport_->topLevelItemCount ();
			i < end; ++i)
	{
		if (Ui_.FeedsToImport_->topLevelItem (i)->checkState (0) !=
				Qt::Checked)
			continue;

		Channel_ptr chan = Channels_ [i];
		feeds_container_t::iterator pos =
			std::find_if (result.begin (), result.end (),
					ParentFinder (chan->ParentURL_));
		if (pos == result.end ())
		{
			Feed_ptr feed (new Feed ());
			feed->URL_ = chan->ParentURL_;
			feed->LastUpdate_ = QDateTime::currentDateTime ();
			feed->Channels_.push_back (chan);
			result.push_back (feed);
		}
		else
			(*pos)->Channels_.push_back (chan);
	}

	return result;
}

void ImportBinary::on_File__textEdited (const QString& newFilename)
{
	Reset ();

	if (QFile (newFilename).exists ())
		Ui_.ButtonBox_->button (QDialogButtonBox::Open)->
			setEnabled (HandleFile (newFilename));
	else
		Reset ();
}

void ImportBinary::on_Browse__released ()
{
	QString startingPath = QFileInfo (Ui_.File_->text ()).path ();
	if (startingPath.isEmpty ())
		startingPath = QDir::homePath ();

	QString filename = QFileDialog::getOpenFileName (this,
			tr ("Select binary file"),
			startingPath,
			tr ("Aggregator exchange files (*.lcae);;"
				"All files (*.*)"));

	if (filename.isEmpty ())
		return;

	Reset ();

	Ui_.File_->setText (filename);

	Ui_.ButtonBox_->button (QDialogButtonBox::Open)->
		setEnabled (HandleFile (filename));
}

bool ImportBinary::HandleFile (const QString& filename)
{
	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		QMessageBox::critical (this,
				tr ("Error"),
				tr ("Could not open file %1 for reading.")
					.arg (filename));
		return false;
	}

	QByteArray buffer = qUncompress (file.readAll ());
	QDataStream stream (&buffer, QIODevice::ReadOnly);

	int magic = 0;
	stream >> magic;
	if (magic != 0xd34df00d)
	{
		QMessageBox::warning (this, tr ("Error"),
				tr ("Selected file %1 is not a valid "
					"LeechCraft::Aggregator exchange file.")
				.arg (filename));
		return false;
	}

	int version = 0;
	stream >> version;

	if (version != 1)
	{
		QMessageBox::warning (this, tr ("Error"),
				tr ("Selected file %1 is a valid LeechCraft::Aggregator "
					"exchange file, but its version %2 is unknown")
				.arg (filename)
				.arg (version));
	}

	QString title, owner, ownerEmail;
	stream >> title >> owner >> ownerEmail;

	while (stream.status () == QDataStream::Ok)
	{
		Channel_ptr channel (new Channel ());
		stream >> (*channel);
		Channels_.push_back (channel);

		QStringList strings (channel->Title_);
		strings << channel->ParentURL_
			<< QString::number (channel->Items_.size ());

		QTreeWidgetItem *item =
			new QTreeWidgetItem (Ui_.FeedsToImport_, strings);

		item->setCheckState (0, Qt::Checked);
	}

	return true;
}

void ImportBinary::Reset ()
{
	Channels_.clear ();
	Ui_.FeedsToImport_->clear ();

	Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);
}

