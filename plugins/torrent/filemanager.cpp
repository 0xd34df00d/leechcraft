#include <QByteArray>
#include <QFile>
#include <QTimer>
#include <QTimerEvent>
#include <QDir>
#include <QtDebug>
#include "filemanager.h"
#include "metainfo.h"

extern "C"
{
#include "sha1/sha1.h"
}

FileManager::FileManager (QObject *parent)
: QThread (parent)
, TotalLength_ (0)
, NumPieces_ (0)
, ReadID_ (0)
, NewFile_ (false)
, StartVerification_ (false)
, Quit_ (false)
, WokeUp_ (false)
{
	VerifiedPieces_.fill (false);
}

FileManager::~FileManager ()
{
	Quit_ = true;
	Condition_.wakeOne ();
	wait ();

	foreach (QFile *file, Files_)
	{
		file->close ();
		delete file;
	}
}

void FileManager::SetMetaInfo (const MetaInfo& info)
{
	MetaInfo_ = info;
}

void FileManager::SetDestinationFolder (const QString& path)
{
	DestinationPath_ = path;
}

int FileManager::Read (int index, int offset, int length)
{
	ReadRequest req;
	req.Index_ = index;
	req.Offset_ = offset;
	req.Length_ = length;

	QMutexLocker locker (&Mutex_);
	req.ID_ = ReadID_++;
	ReadRequests_ << req;
	
	if (!WokeUp_)
	{
		WokeUp_ = true;
		QMetaObject::invokeMethod (this, "wakeUp", Qt::QueuedConnection);
	}
	return req.ID_;
}

void FileManager::Write (int index, int offset, const QByteArray& data)
{
	WriteRequest req;
	req.Index_ = index;
	req.Offset_ = offset;
	req.Data_ = data;

	QMutexLocker locker (&Mutex_);
	WriteRequests_ << req;

	if (!WokeUp_)
	{
		WokeUp_ = true;
		QMetaObject::invokeMethod (this, "wakeUp", Qt::QueuedConnection);
	}
}

void FileManager::VerifyPiece (int index)
{
	QMutexLocker locker (&Mutex_);
	PendingVerificationRequests_ << index;
	StartVerification_ = true;

	if (!WokeUp_)
	{
		WokeUp_ = true;
		QMetaObject::invokeMethod (this, "wakeUp", Qt::QueuedConnection);
	}
}

qint64 FileManager::GetTotalSize () const
{
	return TotalLength_;
}

int FileManager::GetPieceCount () const
{
	return NumPieces_;
}

int FileManager::GetPieceLengthAt (int index) const
{
	QMutexLocker locker (&Mutex_);
	return (Sha1s_.size () == index + 1) ? (TotalLength_ % PieceLength_) : PieceLength_;
}

const QBitArray& FileManager::GetCompletedPieces () const
{
	QMutexLocker locker (&Mutex_);
	return VerifiedPieces_;
}

void FileManager::SetCompletedPieces (const QBitArray& pieces)
{
	QMutexLocker locker (&Mutex_);
	VerifiedPieces_ = pieces;
}

const QString& FileManager::GetErrorString () const
{
	return ErrorString_;
}

void FileManager::startDataVerification ()
{
	QMutexLocker locker (&Mutex_);
	StartVerification_ = true;
	Condition_.wakeOne ();
}

void FileManager::run ()
{
	if (!GenerateFiles ())
		return;

	do
	{
		{
			QMutexLocker locker (&Mutex_);
			if (!Quit_ && ReadRequests_.isEmpty () && WriteRequests_.isEmpty () && !StartVerification_)
				Condition_.wait (&Mutex_);
		}

		Mutex_.lock ();
		QList<ReadRequest> newReadReqs = ReadRequests_;
		ReadRequests_.clear ();
		Mutex_.unlock ();
		while (!newReadReqs.isEmpty ())
		{
			ReadRequest req = newReadReqs.takeFirst ();
			QByteArray block = ReadBlock (req.Index_, req.Offset_, req.Length_);
			emit dataRead (req.ID_, req.Index_, req.Offset_, block);
		}

		Mutex_.lock ();
		QList<WriteRequest> newWriteReqs = WriteRequests_;
		WriteRequests_.clear ();
		while (!Quit_ && !newWriteReqs.isEmpty ())
		{
			WriteRequest req = newWriteReqs.takeFirst ();
//			Mutex_.unlock ();
//			msleep (10);
//			Mutex_.lock ();
			WriteBlock (req.Index_, req.Offset_, req.Data_);
		}

		if (StartVerification_)
		{
			NewPendingVerificationRequests_ = PendingVerificationRequests_;
			PendingVerificationRequests_.clear ();
			VerifyFileContents ();
			StartVerification_ = false;
		}
		Mutex_.unlock ();
		NewPendingVerificationRequests_.clear ();
	} while (!Quit_);

	Mutex_.lock ();
	QList<WriteRequest> newWriteReqs = WriteRequests_;
	WriteRequests_.clear ();
	Mutex_.unlock ();
	while (!newWriteReqs.isEmpty ())
	{
		WriteRequest req = newWriteReqs.takeFirst ();
		WriteBlock (req.Index_, req.Offset_, req.Data_);
	}
}

bool FileManager::GenerateFiles ()
{
	NumPieces_ = -1;

	if (MetaInfo_.GetForm () == MetaInfo::FormSingle)
	{
		QMutexLocker locker (&Mutex_);
		MetaInfoSingleFile misf = MetaInfo_.GetSingleFileInfo ();

		QString prefix;
		if (!DestinationPath_.isEmpty ())
		{
			prefix = DestinationPath_;
			if (!prefix.endsWith ("/"))
				prefix += "/";
			QDir dir;
			if (!dir.mkpath (prefix))
			{
				ErrorString_ = tr ("Failed to create directory %1").arg (prefix);
				emit error ();
				return false;
			}
		}
		QFile *file = new QFile (prefix + misf.Name_);
		if (!file->open (QFile::ReadWrite))
		{
			ErrorString_ = tr ("Failed to open/create file %1: %2").arg (file->fileName ()).arg (file->errorString ());
			emit error ();
			return false;
		}

		if (file->size () != misf.Length_)
		{
			NewFile_ = true;
			if (!file->resize (misf.Length_))
			{
				ErrorString_ = tr ("Failed to resize file %1 to %2 bytes: %3").arg (file->fileName ()).arg (misf.Length_).arg (file->errorString ());
				emit error ();
				return false;
			}
		}
		FileSizes_ << file->size ();
		Files_ << file;
		file->close ();

		PieceLength_ = misf.PieceLength_;
		TotalLength_ = misf.Length_;
		Sha1s_ = misf.Sha1Sums_;
	}
	else
	{
		QMutexLocker locker (&Mutex_);
		QDir dir;
		QString prefix;

		if (!DestinationPath_.isEmpty ())
		{
			prefix = DestinationPath_;
			if (!prefix.endsWith ("/"))
				prefix += "/";
		}
		if (!MetaInfo_.GetName ().isEmpty ())
		{
			prefix += MetaInfo_.GetName ();
			if (!prefix.endsWith ("/"))
				prefix += "/";
		}

		if (!dir.mkpath (prefix))
		{
			ErrorString_ = tr ("Failed to create directory %1").arg (prefix);
			emit error ();
			return false;
		}

		foreach (const MetaInfoMultifile& mimf, MetaInfo_.GetMultiFilesInfo ())
		{
			QString path = QFileInfo (prefix + mimf.Path_).path ();
			if (!QFile::exists (path))
				if (!dir.mkpath (path))
				{
					ErrorString_ = tr ("Failed to create directory %1").arg (path);
					emit error ();
					return false;
				}

			QFile *file = new QFile (prefix + mimf.Path_);
			if (!file->open (QFile::ReadWrite ))
			{
				ErrorString_ = tr ("Failed to open/create file %1: %2").arg (file->fileName ()).arg (file->errorString ());
				emit error ();
				return false;
			}
			if (file->size () != mimf.Length_)
			{
				NewFile_ = true;
				if (!file->resize (mimf.Length_))
				{
					ErrorString_ = tr ("Failed to resize file %1 to %2 bytes: %3").arg (file->fileName ()).arg (mimf.Length_).arg (file->errorString ());
					emit error ();
					return false;
				}
			}

			FileSizes_ << file->size ();
			Files_ << file;
			file->close ();

			TotalLength_ += mimf.Length_;
		}

		Sha1s_ = MetaInfo_.GetSha1Sums ();
		PieceLength_ = MetaInfo_.GetPieceLength ();
	}

	NumPieces_ = Sha1s_.size ();
	return true;
}

QByteArray FileManager::ReadBlock (int index, int offset, int length)
{
	QByteArray block;
	qint64 startReadIndex = static_cast<qint64> (index) * PieceLength_ + offset;
	qint64 currentIndex = 0;

	for (int i = 0; !Quit_ && i < Files_.size () && length > 0; ++i)
	{
		QFile *file = Files_ [i];
		qint64 currentFileSize = FileSizes_ [i];
		if (currentIndex + currentFileSize > startReadIndex)
		{
			if (!file->isOpen ())
				if (!file->open (QFile::ReadWrite))
				{
					ErrorString_ = tr ("Failed to read file %1: %2").arg (file->fileName ()).arg (file->errorString ());
					emit error ();
					break;
				}

			file->seek (startReadIndex - currentIndex);
			QByteArray chunk = file->read (qMin<qint64> (length, currentFileSize - file->pos ()));
			file->close ();

			block += chunk;
			length -= chunk.size ();
			startReadIndex += chunk.size ();
			if (length < 0)
			{
				ErrorString_ = tr ("Failed to read %1 bytes from file %2: %3").arg (length).arg (file->fileName ()).arg (file->errorString ());
				emit error ();
				break;
			}
		}
		currentIndex += currentFileSize;
	}
	return block;
}

bool FileManager::WriteBlock (int index, int offset, const QByteArray& block)
{
	qint64 startWriteIndex = static_cast<qint64> (index) * PieceLength_ + offset;
	qint64 currentIndex = 0;
	int bytesToWrite = block.size ();
	int written = 0;

	for (int i = 0; !Quit_ && i < Files_.size (); ++i)
	{
		QFile *file = Files_ [i];
		qint64 currentFileSize = FileSizes_ [i];

		if (currentIndex + currentFileSize > startWriteIndex)
		{
			if (!file->isOpen ())
				if (!file->open (QFile::ReadWrite))
				{
					ErrorString_ = tr ("Failed to open file %1 for write: %2").arg (file->fileName ()).arg (file->errorString ());
					emit error ();
					break;
				}

			file->seek (startWriteIndex - currentIndex);
			qint64 bytesWritten = file->write (block.constData () + written, qMin<qint64> (bytesToWrite, currentFileSize - file->pos ()));
			file->close ();

			if (bytesWritten <= 0)
			{
				ErrorString_ = tr ("Failed to write to file %1: %2").arg (file->fileName ()).arg (file->errorString ());;
				emit error ();
				return false;
			}

			written += bytesWritten;
			startWriteIndex += bytesWritten;
			bytesToWrite -= bytesWritten;
			if (!bytesToWrite)
				break;
		}
		currentIndex += currentFileSize;
	}
	return true;
}

void FileManager::VerifyFileContents ()
{
	if (NewPendingVerificationRequests_.isEmpty ())
	{
		if (!VerifiedPieces_.count (true))
		{
			VerifiedPieces_.resize (Sha1s_.size ());
			int oldPercent = 0;
			if (!NewFile_)
			{
				int numPieces = Sha1s_.size ();
				for (int i = 0; i < numPieces; ++i)
				{
					verifySinglePiece (i);
					int percent = (i + 1) * 100 / numPieces;
					if (oldPercent != percent)
					{
						emit verificationProgress (percent);
						oldPercent = percent;
					}
				}
			}
		}
		emit verificationDone ();
		return;
	}

	foreach (int index, NewPendingVerificationRequests_)
		emit pieceVerified (index, verifySinglePiece (index));
}

bool FileManager::verifySinglePiece (int index)
{
	QByteArray block = ReadBlock (index, 0, PieceLength_);

	SHA1Context sha;
	SHA1Reset (&sha);
	SHA1Input (&sha, reinterpret_cast<const unsigned char*> (block.constData ()), block.size ());
	SHA1Result (&sha);

	QByteArray sha1sum (20, ' ');
	unsigned char *digest = reinterpret_cast<unsigned char*> (sha.Message_Digest);
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
			sha1sum [i * 4 + j] = digest [i * 4 + j];
#else
			sha1sum [i * 4 + 3 - j] = digest [i * 4 + j];
#endif
		}
	}
	if (sha1sum != Sha1s_ [index])
			return false;

	VerifiedPieces_.setBit (index);
	return true;
}

void FileManager::wakeUp ()
{
	QMutexLocker locker (&Mutex_);
	WokeUp_ = false;
	Condition_.wakeOne ();
}

