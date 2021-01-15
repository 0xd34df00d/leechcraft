/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newtorrentwizard.h"
#include <cmath>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <util/util.h>
#include "newtorrentparams.h"
#include "xmlsettingsmanager.h"
#include "ui_newtorrentfirststep.h"
#include "ui_newtorrentthirdstep.h"

namespace LC::BitTorrent
{
	namespace
	{
		enum Page
		{
			PageIntro,
			PageFirstStep,
			PageSecondStep
		};

		class IntroPage : public QWizardPage
		{
			QLabel *Label_;
		public:
			explicit IntroPage (QWidget *parent = nullptr);
		};

		class FirstStep : public QWizardPage
						, private Ui::NewTorrentFirstStep
		{
		public:
			explicit FirstStep (QWidget *parent = nullptr);

			bool isComplete () const override;
		private:
			QString PrepareDirectory () const;
			void BrowseOutput ();
			void BrowseFile ();
			void BrowseDirectory ();
		};

		class ThirdStep : public QWizardPage
						, private Ui::NewTorrentThirdStep
		{
			quint64 TotalSize_ = 0;
		public:
			explicit ThirdStep (QWidget *parent = nullptr);

			void initializePage () override;
		};
	}

	NewTorrentWizard::NewTorrentWizard (QWidget *parent)
	: QWizard (parent)
	{
		setWindowTitle (tr ("New torrent wizard"));
		setWizardStyle (QWizard::ModernStyle);

		setPage (PageIntro, new IntroPage);
		setPage (PageFirstStep, new FirstStep);
		setPage (PageSecondStep, new ThirdStep);
	}

	NewTorrentParams NewTorrentWizard::GetParams () const
	{
		NewTorrentParams result;

		result.Output_ = field ("Output").toString ();
		result.AnnounceURL_ = field ("AnnounceURL").toString ();
		result.Date_ = field ("Date").toDate ();
		result.Comment_ = field ("Comment").toString ();
		result.Path_ = field ("RootPath").toString ();
		result.URLSeeds_ = field ("URLSeeds").toString ().split (QRegExp("\\s+"));
		result.DHTEnabled_ = field ("DHTEnabled").toBool ();
		result.DHTNodes_ = field ("DHTNodes").toString ().split (QRegExp("\\s+"));
		result.PieceSize_ = 32 * 1024;
		int index = field ("PieceSize").toInt ();
		while (index--)
			result.PieceSize_ *= 2;

		if (result.Path_.endsWith ('/'))
			result.Path_.remove (result.Path_.size () - 1, 1);

		return result;
	}

	namespace
	{
		IntroPage::IntroPage (QWidget *parent)
		: QWizardPage { parent }
		{
			setTitle (tr ("Introduction"));
			Label_ = new QLabel (tr ("This wizard will generate a torrent file. "
									 "You simply need so specify the torrent "
									 "name, files to include and optionally few "
									 "other options to produce your torrent file."));
			Label_->setWordWrap (true);
			const auto lay = new QVBoxLayout;
			lay->addWidget (Label_);
			setLayout (lay);
		}

		FirstStep::FirstStep (QWidget *parent)
		: QWizardPage { parent }
		{
			setupUi (this);
			registerField ("Output", Output_);
			registerField ("AnnounceURL*", AnnounceURL_);
			registerField ("Date", Date_);
			registerField ("Comment", Comment_);
			registerField ("RootPath", RootPath_);
			Date_->setDateTime (QDateTime::currentDateTime ());
			Output_->setText (XmlSettingsManager::Instance ()->property ("LastMakeTorrentDirectory").toString ());
			RootPath_->setText (XmlSettingsManager::Instance ()->property ("LastAddDirectory").toString ());
			connect (RootPath_,
					&QLineEdit::textChanged,
					this,
					&QWizardPage::completeChanged);

			connect (BrowseOutput_,
					&QPushButton::released,
					this,
					&FirstStep::BrowseOutput);
			connect (BrowseFile_,
					&QPushButton::released,
					this,
					&FirstStep::BrowseFile);
			connect (BrowseDirectory_,
					&QPushButton::released,
					this,
					&FirstStep::BrowseDirectory);
		}

		bool FirstStep::isComplete () const
		{
			QFileInfo info { RootPath_->text () };
			return info.exists () &&
					info.isReadable ();
		}

		QString FirstStep::PrepareDirectory () const
		{
			auto directory = RootPath_->text ();
			if (!QFileInfo { directory }.isDir ())
				directory = QFileInfo { directory }.absolutePath ();

			if (!QFileInfo::exists (directory))
				directory = QDir::homePath ();

			if (!directory.endsWith ('/'))
				directory.append ('/');

			return directory;
		}

		void FirstStep::BrowseOutput ()
		{
			auto last = XmlSettingsManager::Instance ()->property ("LastMakeTorrentDirectory").toString ();
			if (!last.endsWith ('/'))
				last += '/';
			if (!QFileInfo::exists (last))
				last = QDir::homePath ();

			const auto& directory = QFileDialog::getSaveFileName (this,
					tr ("Select where to save torrent file"),
					last);
			if (directory.isEmpty ())
				return;

			Output_->setText (directory);
			XmlSettingsManager::Instance ()->setProperty ("LastMakeTorrentDirectory",
					QFileInfo { directory }.absolutePath ());
		}

		void FirstStep::BrowseFile ()
		{
			const auto& path = QFileDialog::getOpenFileName (this,
					tr ("Select torrent contents"),
					PrepareDirectory ());
			if (path.isEmpty ())
				return;

			RootPath_->setText (path);
			XmlSettingsManager::Instance ()->setProperty ("LastAddDirectory",
					QFileInfo { path }.absolutePath ());

			emit completeChanged ();
		}

		void FirstStep::BrowseDirectory ()
		{
			const auto& path = QFileDialog::getExistingDirectory (this,
					tr ("Select torrent contents"),
					PrepareDirectory ());
			if (path.isEmpty ())
				return;

			RootPath_->setText (path);
			XmlSettingsManager::Instance ()->setProperty ("LastAddDirectory", path);

			emit completeChanged ();
		}

		ThirdStep::ThirdStep (QWidget *parent)
		: QWizardPage { parent }
		{
			setupUi (this);
			registerField ("PieceSize", PieceSize_);
			registerField ("URLSeeds", URLSeeds_);
			registerField ("DHTEnabled", DHTEnabled_);
			registerField ("DHTNodes", DHTNodes_);

			connect (PieceSize_,
					qOverload<int> (&QComboBox::currentIndexChanged),
					[this]
					{
						quint32 mul = 32 * 1024;
						int index = PieceSize_->currentIndex ();
						while (index--)
							mul *= 2;

						int numPieces = TotalSize_ / mul;
						if (TotalSize_ % mul)
							++numPieces;

						NumPieces_->setText (QString::number (numPieces) +
								tr (" pieces (%1)")
										.arg (Util::MakePrettySize (TotalSize_)));
					});
		}

		void ThirdStep::initializePage ()
		{
			TotalSize_ = 0;
			QString path = field ("RootPath").toString ();

			QFileInfo pathInfo (path);
			if (pathInfo.isDir ())
			{
				QDirIterator it (path,
						QDirIterator::Subdirectories);
				while (it.hasNext ())
				{
					it.next ();
					QFileInfo info = it.fileInfo ();
					if (info.isFile () && info.isReadable ())
						TotalSize_ += info.size ();
				}
			}
			else if (pathInfo.isFile () &&
					pathInfo.isReadable ())
				TotalSize_ += pathInfo.size ();

			quint64 max = std::log (static_cast<long double> (TotalSize_ / 102400)) * 80;

			quint32 pieceSize = 32 * 1024;
			int shouldIndex = 0;
			for (; TotalSize_ / pieceSize >= max; pieceSize *= 2, ++shouldIndex) ;

			if (shouldIndex > PieceSize_->count () - 1)
				shouldIndex = PieceSize_->count () - 1;

			PieceSize_->setCurrentIndex (shouldIndex);
		}
	}
}
