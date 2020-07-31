/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <util/util.h>
#include "../core.h"

namespace LC
{
namespace LMP
{
	class ISyncPlugin;

	class CopyManagerBase : public QObject
	{
		Q_OBJECT
	protected:
		using QObject::QObject;
	protected slots:
		virtual void handleUploadFinished (const QString& localPath,
				QFile::FileError error, const QString& errorStr) = 0;
	signals:
		void startedCopying (const QString&);
		void copyProgress (qint64, qint64);
		void finishedCopying ();
		void errorCopying (const QString&, const QString&);
	};

	template<typename CopyJobT>
	class CopyManager : public CopyManagerBase
	{
		QList<CopyJobT> Queue_;
		CopyJobT CurrentJob_;
	public:
		CopyManager (QObject *parent = 0)
		: CopyManagerBase (parent)
		{
		}

		void Copy (const CopyJobT& job)
		{
			if (IsRunning ())
				Queue_ << job;
			else
				StartJob (job);
		}
	private:
		void StartJob (const CopyJobT& job)
		{
			CurrentJob_ = job;

			connect (job.GetQObject (),
					SIGNAL (uploadFinished (QString, QFile::FileError, QString)),
					this,
					SLOT (handleUploadFinished (QString, QFile::FileError, QString)),
					Qt::UniqueConnection);

			const auto& norm = QMetaObject::normalizedSignature ("uploadProgress (qint64, qint64)");
			if (job.GetQObject ()->metaObject ()->indexOfSignal (norm) >= 0)
				connect (job.GetQObject (),
						SIGNAL (uploadProgress (qint64, qint64)),
						this,
						SIGNAL (copyProgress (qint64, qint64)));

			job.Upload ();

			emit startedCopying (job.Filename_);
		}

		bool IsRunning () const
		{
			return !CurrentJob_.Filename_.isEmpty ();
		}
	protected:
		void handleUploadFinished (const QString& localPath, QFile::FileError error, const QString& errorStr) override
		{
			const bool remove = CurrentJob_.RemoveOnFinish_;
			CurrentJob_ = CopyJobT ();

			if (!Queue_.isEmpty ())
				StartJob (Queue_.takeFirst ());

			if (remove)
				QFile::remove (localPath);

			if (!errorStr.isEmpty () && error != QFile::NoError)
				emit errorCopying (localPath, errorStr);
			else
				emit finishedCopying ();
		}
	};
}
}
