/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartmanagerdialog.h"
#include <chrono>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QFileDialog>
#include <QTimer>
#include <QtConcurrentMap>
#include <util/threads/futures.h>
#include "albumartmanager.h"

namespace LC::LMP
{
	namespace
	{
		class ReturnPressSwallower : public QObject
		{
			AlbumArtManagerDialog& Dia_;
		public:
			explicit ReturnPressSwallower (AlbumArtManagerDialog& dia)
			: QObject { &dia }
			, Dia_ { dia }
			{
			}

			bool eventFilter (QObject*, QEvent *e) override
			{
				if (e->type () != QEvent::KeyPress)
					return false;

				auto keyEv = static_cast<QKeyEvent*> (e);
				if (keyEv->key () == Qt::Key_Enter || keyEv->key () == Qt::Key_Return)
				{
					Dia_.ScheduleRequest ();
					return true;
				}

				return false;
			}
		};
	}

	AlbumArtManagerDialog::AlbumArtManagerDialog (int albumId,
			const QString& artist,
			const QString& album,
			AlbumArtManager *aamgr,
			QWidget *parent)
	: QDialog { parent }
	, AAMgr_ { aamgr }
	, Model_ { new QStandardItemModel { this } }
	, AlbumID_ { albumId }
	, Artist_ { artist }
	, Album_ { album }
	, ReqTimer_ { new QTimer { this } }
	{
		ReqTimer_->setSingleShot (true);
		ReqTimer_->callOnTimeout (this, &AlbumArtManagerDialog::Request);

		Ui_.setupUi (this);
		Ui_.ArtistLine_->setText (artist);
		Ui_.AlbumLine_->setText (album);

		auto swallower = new ReturnPressSwallower { *this };
		Ui_.ArtistLine_->installEventFilter (swallower);
		Ui_.AlbumLine_->installEventFilter (swallower);

		Ui_.ArtView_->setModel (Model_);

		if (!Artist_.isEmpty () && !Album_.isEmpty ())
			setWindowTitle (tr ("Album art for %1 — %2")
					.arg (Artist_.trimmed (), Album_.trimmed ()));

		connect (Ui_.ArtistLine_,
				&QLineEdit::textChanged,
				this,
				&AlbumArtManagerDialog::ScheduleRequest);
		connect (Ui_.AlbumLine_,
				&QLineEdit::textChanged,
				this,
				&AlbumArtManagerDialog::ScheduleRequest);

		connect (Ui_.ArtView_,
				&QAbstractItemView::activated,
				this,
				&QDialog::accept);

		connect (Ui_.BrowseButton_,
				&QPushButton::released,
				this,
				&AlbumArtManagerDialog::BrowseImage);

		ScheduleRequest ();
	}

	QString AlbumArtManagerDialog::GetArtist () const
	{
		return Ui_.ArtistLine_->text ();
	}

	QString AlbumArtManagerDialog::GetAlbum () const
	{
		return Ui_.AlbumLine_->text ();
	}

	void AlbumArtManagerDialog::accept ()
	{
		if (const auto& idx = Ui_.ArtView_->currentIndex ();
			idx.isValid ())
			AAMgr_->SetAlbumArt (AlbumID_, Artist_, Album_, FullImages_ [idx.row ()]);

		QDialog::accept ();
	}

	void AlbumArtManagerDialog::BrowseImage ()
	{
		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Choose album art"),
				QDir::homePath (),
				tr ("Images (*.png *.jpg *.jpeg);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		QImage image (filename);
		if (image.isNull ())
			return;

		HandleImages ({ image });
	}

	void AlbumArtManagerDialog::HandleImages (const QList<QImage>& images)
	{
		struct ScaleResult
		{
			QImage Image_;
			QImage SourceImage_;
		};

		auto watcher = new QFutureWatcher<ScaleResult> { this };

		auto worker = [] (const QImage& image) -> ScaleResult
		{
			return { image.scaled (200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation), image };
		};
		watcher->setFuture (QtConcurrent::mapped (images, std::function<ScaleResult (QImage)> (worker)));

		connect (watcher,
				&QFutureWatcher<ScaleResult>::finished,
				this,
				[this, watcher]
				{
					for (const auto& result : watcher->future ())
					{
						auto item = new QStandardItem ();
						item->setIcon (QIcon (QPixmap::fromImage (result.Image_)));
						item->setText (QStringLiteral ("%1×%2")
								.arg (result.SourceImage_.width ())
								.arg (result.SourceImage_.height ()));
						item->setEditable (false);
						Model_->appendRow (item);

						FullImages_ << result.SourceImage_;
					}

					watcher->deleteLater ();
				});
	}

	void AlbumArtManagerDialog::Request ()
	{
		const auto& artist = GetArtist ();
		const auto& album = GetAlbum ();
		if (artist.isEmpty () || album.isEmpty ())
			return;

		Util::Sequence (this, AAMgr_->CheckAlbumArt (artist, album)) >>
				[this, artist, album] (const QList<QImage>& images)
				{
					if (GetArtist () == artist && GetAlbum () == album)
						HandleImages (images);
				};
	}

	void AlbumArtManagerDialog::ScheduleRequest ()
	{
		Model_->clear ();

		using namespace std::chrono_literals;
		ReqTimer_->start (1s);
	}
}
