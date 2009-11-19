/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef NUFELLA_H
#define NUFELLA_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>

class Nufella : public QObject
			  , public IInfo
			  , public IDownload
			  , public IJobHolder
{
	Q_OBJECT

	Q_INTERFACES (IInfo IDownload IJobHolder)

	std::auto_ptr<QTranslator> Translator_;
public:
	void Init (ICoreProxy_ptr);
	void SecondInit ();
	void Release ();
	QString GetName () const;
	QString GetInfo () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	QIcon GetIcon () const;
	qint64 GetDownloadSpeed () const;
	qint64 GetUploadSpeed () const;
	void StartAll ();
	void StopAll ();
	bool CouldDownload (const LeechCraft::DownloadEntity&) const;
	int AddJob (LeechCraft::DownloadEntity);
	QAbstractItemModel* GetRepresentation () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;
};

#endif

