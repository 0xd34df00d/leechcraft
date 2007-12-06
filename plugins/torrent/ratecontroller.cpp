#include <QtCore>
#include "ratecontroller.h"
#include "peerconnection.h"

Q_GLOBAL_STATIC (RateController, RateControllerInstance);

RateController::RateController (QObject *parent)
: QObject (parent)
, TransferScheduled_ (false)
{
}

RateController* RateController::Instance ()
{
	return RateControllerInstance ();
}

void RateController::AddSocket (PeerConnection *pc)
{
	connect (pc, SIGNAL (readyToTransfer ()), this, SLOT (scheduleTransfer ()));
	pc->setReadBufferSize (DownLimit_ * 4);
	Sockets_ << pc;
	scheduleTransfer ();
}

void RateController::RemoveSocket (PeerConnection *pc)
{
	disconnect (pc, SIGNAL (readyToTransfer ()), this, SLOT (scheduleTransfer ()));
	pc->setReadBufferSize (0);
	Sockets_.remove (pc);
}

int RateController::GetUploadLimit () const
{
	return UpLimit_;
}

int RateController::GetDownloadLimit () const
{
	return DownLimit_;
}

void RateController::SetUploadLimit (int val)
{
	UpLimit_ = val;
}

void RateController::SetDownloadLimit (int val)
{
	DownLimit_ = val;
	foreach (PeerConnection *pc, Sockets_)
		pc->setReadBufferSize (DownLimit_ * 4);
}

void RateController::transfer ()
{
	TransferScheduled_ = false;
	if (Sockets_.isEmpty ())
		return;

	int msecs = 1000;
	if (!StopWatch_.isNull ())
		msecs = qMin (msecs, StopWatch_.elapsed ());

	qint64 bytesToRead = (DownLimit_ * msecs) / 1000;
	qint64 bytesToWrite = (UpLimit_ * msecs) / 1000;
	if (!bytesToRead && !bytesToWrite)
	{
		scheduleTransfer ();
		return;
	}

	QSet<PeerConnection*> pending;
	foreach (PeerConnection *client, Sockets_)
	{
		if (client->CanTransfer ())
			pending << client;
	}
	if (pending.isEmpty ())
		return;

	StopWatch_.start ();

	bool can;
	do
	{
		can = false;
		qint64 write = qMax<qint64> (1, bytesToWrite / pending.size ());
		qint64 read = qMax<qint64> (1, bytesToRead / pending.size ());

		QSetIterator<PeerConnection*> it (pending);
		while (it.hasNext () && (bytesToWrite > 0 || bytesToRead > 0))
		{
			PeerConnection *socket = it.next ();
			if (socket->state () != QAbstractSocket::ConnectedState)
			{
				pending.remove (socket);
				continue;
			}

			bool transferred = false;
			qint64 avail = qMin<qint64> (socket->GetSocketBytes (), read);
			if (avail > 0)
			{
				qint64 readBytes = socket->Read (qMin<qint64> (avail, bytesToRead));
				if (readBytes > 0)
				{
					bytesToRead -= readBytes;
					transferred = true;
				}
			}

			if (UpLimit_ * 2 > socket->GetBytesToWrite ())
			{
				qint64 chunkSize = qMin<qint64> (write, bytesToWrite);
				qint64 toWrite = qMin<qint64> (UpLimit_ * 2 - socket->GetBytesToWrite (), chunkSize);
				if (toWrite > 0)
				{
					qint64 writtenBytes = socket->Write (toWrite);
					if (writtenBytes > 0)
					{
						bytesToWrite -= writtenBytes;
						transferred = true;
					}
				}
			}

			if (transferred && socket->CanTransfer ())
				can = true;
			else
				pending.remove (socket);
		}
	} while (can && (bytesToWrite > 0 || bytesToRead > 0) && !pending.isEmpty ());

	if (can || !bytesToRead || !bytesToWrite)
		scheduleTransfer ();
}

void RateController::scheduleTransfer ()
{
	if (TransferScheduled_)
		return;
	TransferScheduled_ = true;
	QTimer::singleShot (50, this, SLOT (transfer ()));
}

