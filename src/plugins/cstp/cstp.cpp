/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "cstp.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/logic/tribool.hpp>
#include <QMenu>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QTabWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QModelIndex>
#include <QDir>
#include <QUrl>
#include <QTextCodec>
#include <QTranslator>
#include <QMainWindow>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace CSTP
{
	CSTP::~CSTP ()
	{
	}

	void CSTP::Init (ICoreProxy_ptr coreProxy)
	{
		Core::Instance ().SetCoreProxy (coreProxy);
		Translator_.reset (Util::InstallTranslator ("cstp"));

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"cstpsettings.xml");

		SetupToolbar ();

		Core::Instance ().SetToolbar (Toolbar_.get ());

		connect (&Core::Instance (),
				SIGNAL (taskFinished (int)),
				this,
				SIGNAL (jobFinished (int)));
		connect (&Core::Instance (),
				SIGNAL (taskRemoved (int)),
				this,
				SIGNAL (jobRemoved (int)));
		connect (&Core::Instance (),
				SIGNAL (taskError (int, IDownload::Error)),
				this,
				SIGNAL (jobError (int, IDownload::Error)));
		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (error (const QString&)),
				this,
				SLOT (handleError (const QString&)));
	}

	void CSTP::SecondInit ()
	{
	}

	void CSTP::Release ()
	{
		Core::Instance ().Release ();
		XmlSettingsManager::Instance ().Release ();
		XmlSettingsDialog_.reset ();
		Toolbar_.reset ();
		Translator_.reset ();
	}

	QByteArray CSTP::GetUniqueID () const
	{
		return "org.LeechCraft.CSTP";
	}

	QString CSTP::GetName () const
	{
		return "CSTP";
	}

	QString CSTP::GetInfo () const
	{
		return "Common Stream Transfer Protocols";
	}

	QStringList CSTP::Provides () const
	{
		return QStringList ("http") << "https" << "remoteable" << "resume";
	}

	QStringList CSTP::Needs () const
	{
		return QStringList ();
	}

	QStringList CSTP::Uses () const
	{
		return QStringList ();
	}

	void CSTP::SetProvider (QObject*, const QString&)
	{
	}

	QIcon CSTP::GetIcon () const
	{
		return QIcon (":/plugins/cstp/resources/images/cstp.svg");
	}

	qint64 CSTP::GetDownloadSpeed () const
	{
		return Core::Instance ().GetTotalDownloadSpeed ();
	}

	qint64 CSTP::GetUploadSpeed () const
	{
		return 0;
	}

	void CSTP::StartAll ()
	{
		Core::Instance ().startAllTriggered ();
	}

	void CSTP::StopAll ()
	{
		Core::Instance ().stopAllTriggered ();
	}

	EntityTestHandleResult CSTP::CouldDownload (const LeechCraft::Entity& e) const
	{
		return Core::Instance ().CouldDownload (e);
	}

	int CSTP::AddJob (LeechCraft::Entity e)
	{
		return Core::Instance ().AddTask (e);
	}

	void CSTP::KillTask (int id)
	{
		Core::Instance ().KillTask (id);
	}

	QAbstractItemModel* CSTP::GetRepresentation () const
	{
		return Core::Instance ().GetRepresentationModel ();
	}

	void CSTP::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& si, const QModelIndex&)
	{
		QModelIndex index = Core::Instance ().GetCoreProxy ()->MapToSource (si);
		if (index.model () != GetRepresentation ())
			index = QModelIndex ();
		Core::Instance ().ItemSelected (index);
	}

	Util::XmlSettingsDialog_ptr CSTP::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	template<typename T>
	void CSTP::ApplyCore2Selection (void (Core::*temp) (const QModelIndex&), T view)
	{
		QModelIndexList indexes = view->selectionModel ()->
			selectedRows ();
		std::for_each (indexes.begin (), indexes.end (),
				boost::bind (temp,
					&Core::Instance (),
					_1));
	}

	void CSTP::SetupToolbar ()
	{
		Toolbar_.reset (new QToolBar);
		Toolbar_->setWindowTitle ("CSTP");

		QAction *remove = Toolbar_->addAction (tr ("Remove"));
		connect (remove,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (removeTriggered ()));
		remove->setProperty ("ActionIcon", "list-remove");

		QAction *removeAll = Toolbar_->addAction (tr ("Remove all"));
		connect (removeAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (removeAllTriggered ()));
		removeAll->setProperty ("ActionIcon", "edit-clear-list");

		Toolbar_->addSeparator ();

		QAction *start = Toolbar_->addAction (tr ("Start"));
		connect (start,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (startTriggered ()));
		start->setProperty ("ActionIcon", "media-playback-start");

		QAction *stop = Toolbar_->addAction (tr ("Stop"));
		connect (stop,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (stopTriggered ()));
		stop->setProperty ("ActionIcon", "media-playback-stop");

		QAction *startAll = Toolbar_->addAction (tr ("media-seek-forward"));
		connect (startAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (startAllTriggered ()));
		startAll->setProperty ("ActionIcon", "cstp_startall");

		QAction *stopAll = Toolbar_->addAction (tr ("media-record"));
		connect (stopAll,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (stopAllTriggered ()));
		stopAll->setProperty ("ActionIcon", "cstp_stopall");
	}

	void CSTP::handleFileExists (boost::logic::tribool *remove)
	{
		QMessageBox::StandardButton userReply =
			QMessageBox::warning (Core::Instance ().GetCoreProxy ()->GetMainWindow (),
				tr ("File exists"),
				tr ("File %1 already exists, continue download?"),
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (userReply == QMessageBox::Yes)
			*remove = false;
		else if (userReply == QMessageBox::No)
			*remove = true;
		else
			*remove = boost::logic::indeterminate;
	}

	void CSTP::handleError (const QString& error)
	{
		emit gotEntity (Util::MakeNotification ("HTTP error", error, PCritical_));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_cstp, LeechCraft::CSTP::CSTP);
