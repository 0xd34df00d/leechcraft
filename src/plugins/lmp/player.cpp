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

#include "player.h"
#include <QToolBar>
#include <QStatusBar>
#include <QSlider>
#include <QMessageBox>
#include <QStandardItem>
#include <QUrl>
#include "keyinterceptor.h"
#include "core.h"
#include "xmlsettingsmanager.h"

Q_DECLARE_METATYPE (Phonon::MediaSource*);

using namespace Phonon;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			Player::Player (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				StatusBar_ = new QStatusBar (this);
				layout ()->addWidget (StatusBar_);

				QueueModel_.reset (new QStandardItemModel);
				QueueModel_->setHorizontalHeaderLabels (QStringList (tr ("Media source"))
						<< tr ("Source type"));
				Ui_.Queue_->setModel (QueueModel_.get ());

				connect (Ui_.Player_->GetMediaObject (),
						SIGNAL (hasVideoChanged (bool)),
						Ui_.Player_,
						SLOT (setVisible (bool)));
				connect (Ui_.Player_->GetMediaObject (),
						SIGNAL (currentSourceChanged (const Phonon::MediaSource&)),
						this,
						SLOT (handleSourceChanged (const Phonon::MediaSource&)));

				KeyInterceptor *ki = new KeyInterceptor (Ui_.Player_, this);
				QList<QWidget*> children = findChildren<QWidget*> ();
				children << Ui_.Player_;
				for (QList<QWidget*>::iterator i = children.begin (),
						end = children.end (); i != end; ++i)
					(*i)->installEventFilter (ki);

				connect (Ui_.Player_,
						SIGNAL (stateUpdated (const QString&)),
						this,
						SLOT (handleStateUpdated (const QString&)));
				connect (Ui_.Player_,
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));
			}

			void Player::Play ()
			{
				Ui_.Player_->Play ();
			}

			void Player::Pause ()
			{
				Ui_.Player_->Pause ();
			}

			void Player::TogglePause ()
			{
				Ui_.Player_->togglePause ();
			}

			void Player::Enqueue (MediaSource *source)
			{
				QList<QStandardItem*> items;
				switch (source->type ())
				{
					case MediaSource::LocalFile:
						items << new QStandardItem (source->fileName ())
							<< new QStandardItem (tr ("File"));
						break;
					case MediaSource::Url:
						items << new QStandardItem (source->url ().toString ())
							<< new QStandardItem (tr ("URL"));
						break;
					case MediaSource::Disc:
						items << new QStandardItem (source->deviceName ());
						switch (source->discType ())
						{
							case Cd:
								items << new QStandardItem (tr ("Audio CD"));
								break;
							case Dvd:
								items << new QStandardItem (tr ("DVD"));
								break;
							case Vcd:
								items << new QStandardItem (tr ("Video CD"));
								break;
							default:
								items << new QStandardItem (tr ("Unknown disc type"));
								break;
						}
						break;
					case MediaSource::Stream:
						items << new QStandardItem (source->fileName ())
							<< new QStandardItem (tr ("Stream"));
						break;
					case MediaSource::Invalid:
#if PHONON_VERSION >= PHONON_VERSION_CHECK (4, 3, 0)
					case MediaSource::Empty:
#endif
						return;
				}

				items.at (0)->setData (QVariant::fromValue<MediaSource*> (source), SourceRole);

				QueueModel_->appendRow (items);
				Ui_.Player_->Enqueue (*source);
			}

			void Player::FillQueue (int start) const
			{
				for (int i = start; i < QueueModel_->rowCount (); ++i)
					Ui_.Player_->GetMediaObject ()->
						enqueue (*QueueModel_->item (i)->
								data (SourceRole).value<MediaSource*> ());
			}

			void Player::handleStateUpdated (const QString& state)
			{
				StatusBar_->showMessage (state);
			}

			void Player::handleError (const QString& error)
			{
				QMessageBox::warning (this,
						tr ("LeechCraft"),
						error);
			}

			void Player::handleSourceChanged (const Phonon::MediaSource& source)
			{
				for (int i = 0; i < QueueModel_->rowCount (); ++i)
				{
					QStandardItem *item = QueueModel_->item (i);
					if (source == *item->data (SourceRole).value<MediaSource*> ())
						item->setIcon (Core::Instance ().GetCoreProxy ()->GetIcon ("lmp_play"));
					else
						item->setIcon (QIcon ());
				}
			}

			void Player::on_Queue__activated (const QModelIndex& si)
			{
				MediaObject *o = Ui_.Player_->GetMediaObject ();

				MediaSource *source = QueueModel_->
					item (si.row ())->data (SourceRole).value<MediaSource*> ();
				o->clearQueue ();
				o->setCurrentSource (*source);
				o->play ();
				FillQueue (si.row ());
			}
		};
	};
};

