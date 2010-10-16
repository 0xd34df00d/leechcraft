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

#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <QMessageBox>
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QCompleter>
#include <QSystemTrayIcon>
#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QQueue>
#include <QTimer>
#include <QTranslator>
#include <QCursor>
#include <QKeyEvent>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/util.h>
#include <plugininterface/categoryselector.h>
#include <plugininterface/tagscompleter.h>
#include <plugininterface/backendselector.h>
#include <plugininterface/flattofoldersproxymodel.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_mainwidget.h"
#include "itemsfiltermodel.h"
#include "channelsfiltermodel.h"
#include "aggregator.h"
#include "core.h"
#include "addfeed.h"
#include "itemsfiltermodel.h"
#include "channelsfiltermodel.h"
#include "xmlsettingsmanager.h"
#include "regexpmatcherui.h"
#include "regexpmatchermanager.h"
#include "export.h"
#include "importbinary.h"
#include "feedsettings.h"
#include "jobholderrepresentation.h"
#include "wizardgenerator.h"
#include "export2fb2dialog.h"
#include "actionsstructs.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			using LeechCraft::Util::TagsCompleter;
			using LeechCraft::Util::CategorySelector;
			using LeechCraft::ActionInfo;

			struct Aggregator_Impl
			{
				Ui::MainWidget Ui_;

				AppWideActions AppWideActions_;
				ChannelActions ChannelActions_;

				QMenu *ToolMenu_;

				boost::shared_ptr<Util::FlatToFoldersProxyModel> FlatToFolders_;
				boost::shared_ptr<Util::XmlSettingsDialog> XmlSettingsDialog_;
				std::auto_ptr<Util::TagsCompleter> TagsLineCompleter_;
				std::auto_ptr<QSystemTrayIcon> TrayIcon_;
				std::auto_ptr<QTranslator> Translator_;
				std::auto_ptr<RegexpMatcherUi> RegexpMatcherUi_;

				QModelIndex SelectedRepr_;

				enum
				{
					EAActionAddFeed_,
					EAActionUpdateFeeds_,
					EAActionRemoveFeed_,
					EAActionMarkChannelAsRead_,
					EAActionMarkChannelAsUnread_,
					EAActionChannelSettings_,
					EAActionUpdateSelectedFeed_,
					EAActionUNUSED1_,
					EAActionRegexpMatcher_,
					EAActionHideReadItems_,
					EAActionImportOPML_,
					EAActionExportOPML_,
					EAActionImportBinary_,
					EAActionExportBinary_,
					EAActionExportFB2_
				};
			};

			Aggregator::~Aggregator ()
			{
			}

			void Aggregator::Init (ICoreProxy_ptr proxy)
			{
				setProperty ("IsUnremoveable", true);

				Impl_ = new Aggregator_Impl;
				Impl_->Translator_.reset (LeechCraft::Util::InstallTranslator ("aggregator"));

				Impl_->ChannelActions_.SetupActionsStruct (this);
				Impl_->AppWideActions_.SetupActionsStruct (this);
				Core::Instance ().SetAppWideActions (Impl_->AppWideActions_);

				Impl_->ToolMenu_ = new QMenu (tr ("Aggregator"));
				Impl_->ToolMenu_->setIcon (GetIcon ());
				Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionImportOPML_);
				Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionExportOPML_);
				Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionImportBinary_);
				Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionExportBinary_);
				Impl_->ToolMenu_->addAction (Impl_->AppWideActions_.ActionExportFB2_);

				Impl_->TrayIcon_.reset (new QSystemTrayIcon (QIcon (":/resources/images/aggregator.svg"), this));
				Impl_->TrayIcon_->hide ();
				connect (Impl_->TrayIcon_.get (),
						SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
						this,
						SLOT (trayIconActivated ()));

				Core::Instance ().SetProxy (proxy);

				connect (&Core::Instance (),
						SIGNAL (unreadNumberChanged (int)),
						this,
						SLOT (unreadNumberChanged (int)));
				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));

				Impl_->XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				Impl_->XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"aggregatorsettings.xml");
				Impl_->XmlSettingsDialog_->SetCustomWidget ("BackendSelector",
						new LeechCraft::Util::BackendSelector (XmlSettingsManager::Instance ()));

				bool initFailed = false;
				if (!Core::Instance ().DoDelayedInit ())
				{
					setEnabled (false);
					Impl_->AppWideActions_.ActionAddFeed_->setEnabled (false);
					Impl_->AppWideActions_.ActionUpdateFeeds_->setEnabled (false);
					Impl_->AppWideActions_.ActionRegexpMatcher_->setEnabled (false);
					Impl_->AppWideActions_.ActionImportOPML_->setEnabled (false);
					Impl_->AppWideActions_.ActionExportOPML_->setEnabled (false);
					Impl_->AppWideActions_.ActionImportBinary_->setEnabled (false);
					Impl_->AppWideActions_.ActionExportBinary_->setEnabled (false);
					Impl_->AppWideActions_.ActionExportFB2_->setEnabled (false);
					initFailed = true;
					qWarning () << Q_FUNC_INFO
						<< "core initialization failed";
				}

				Impl_->Ui_.setupUi (this);
				Impl_->Ui_.ItemsWidget_->SetChannelActions (Impl_->ChannelActions_);

				if (initFailed)
				{
					QMessageBox::critical (this,
							"LeechCraft",
							tr ("Aggregator failed to initialize properly. "
								"Check logs and talk with the developers. "
								"Or, at least, check the storage backend "
								"settings and restart LeechCraft."));
					return;
				}

				Impl_->Ui_.ItemsWidget_->
					SetChannelsFilter (Core::Instance ()
							.GetChannelsModel ());
				Core::Instance ().GetJobHolderRepresentation ()->setParent (this);
				Core::Instance ().GetReprWidget ()->SetChannelActions (Impl_->ChannelActions_);

				Impl_->Ui_.MergeItems_->setChecked (XmlSettingsManager::Instance ()->
						Property ("MergeItems", false).toBool ());

				Impl_->RegexpMatcherUi_.reset (new RegexpMatcherUi (this));

				Impl_->FlatToFolders_.reset (new Util::FlatToFoldersProxyModel);
				Impl_->FlatToFolders_->SetTagsManager (Core::Instance ().GetProxy ()->GetTagsManager ());
				handleGroupChannels ();
				connect (Impl_->FlatToFolders_.get (),
						SIGNAL (rowsInserted (const QModelIndex&,
								int, int)),
						Impl_->Ui_.Feeds_,
						SLOT (expandAll ()));
				connect (Impl_->FlatToFolders_.get (),
						SIGNAL (rowsRemoved (const QModelIndex&,
								int, int)),
						Impl_->Ui_.Feeds_,
						SLOT (expandAll ()));
				XmlSettingsManager::Instance ()->RegisterObject ("GroupChannelsByTags",
						this, "handleGroupChannels");

				Impl_->Ui_.Feeds_->addAction (Impl_->
						ChannelActions_.ActionMarkChannelAsRead_);
				Impl_->Ui_.Feeds_->addAction (Impl_->
						ChannelActions_.ActionMarkChannelAsUnread_);
				QAction *sep1 = new QAction (Impl_->Ui_.Feeds_),
						*sep2 = new QAction (Impl_->Ui_.Feeds_);
				sep1->setSeparator (true);
				sep2->setSeparator (true);
				Impl_->Ui_.Feeds_->addAction (sep1);
				Impl_->Ui_.Feeds_->addAction (Impl_->
						ChannelActions_.ActionRemoveFeed_);
				Impl_->Ui_.Feeds_->addAction (Impl_->
						ChannelActions_.ActionUpdateSelectedFeed_);
				Impl_->Ui_.Feeds_->addAction (sep2);
				Impl_->Ui_.Feeds_->addAction (Impl_->
						ChannelActions_.ActionChannelSettings_);
				Impl_->Ui_.Feeds_->setContextMenuPolicy (Qt::ActionsContextMenu);
				QHeaderView *channelsHeader = Impl_->Ui_.Feeds_->header ();

				QMenu *contextMenu = new QMenu (tr ("Feeds actions"));
				contextMenu->addAction (Impl_->
						ChannelActions_.ActionMarkChannelAsRead_);
				contextMenu->addAction (Impl_->
						ChannelActions_.ActionMarkChannelAsUnread_);
				contextMenu->addSeparator ();
				contextMenu->addAction (Impl_->
						ChannelActions_.ActionRemoveFeed_);
				contextMenu->addAction (Impl_->
						ChannelActions_.ActionUpdateSelectedFeed_);
				contextMenu->addSeparator ();
				contextMenu->addAction (Impl_->
						ChannelActions_.ActionChannelSettings_);
				Core::Instance ().SetContextMenu (contextMenu);

				QFontMetrics fm = fontMetrics ();
				int dateTimeSize = fm.width (QDateTime::currentDateTime ()
						.toString (Qt::SystemLocaleShortDate) + "__");
				channelsHeader->resizeSection (0, fm.width ("Average channel name"));
				channelsHeader->resizeSection (1, fm.width ("_9999_"));
				channelsHeader->resizeSection (2, dateTimeSize);
				connect (Impl_->Ui_.TagsLine_,
						SIGNAL (textChanged (const QString&)),
						Core::Instance ().GetChannelsModel (),
						SLOT (setFilterFixedString (const QString&)));
				connect (Impl_->AppWideActions_.ActionUpdateFeeds_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (updateFeeds ()));

				Impl_->TagsLineCompleter_.reset (new TagsCompleter (Impl_->Ui_.TagsLine_));
				Impl_->Ui_.TagsLine_->AddSelector ();

				Impl_->Ui_.MainSplitter_->setStretchFactor (0, 5);
				Impl_->Ui_.MainSplitter_->setStretchFactor (1, 9);

				connect (&RegexpMatcherManager::Instance (),
						SIGNAL (gotLink (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));

				currentChannelChanged ();
			}

			void Aggregator::SecondInit ()
			{
			}

			void Aggregator::Release ()
			{
				disconnect (&Core::Instance (), 0, this, 0);
				if (Core::Instance ().GetChannelsModel ())
					disconnect (Core::Instance ().GetChannelsModel (), 0, this, 0);
				if (Impl_->TagsLineCompleter_.get ())
					disconnect (Impl_->TagsLineCompleter_.get (), 0, this, 0);
				Impl_->TrayIcon_->hide ();
				delete Impl_;
				Core::Instance ().Release ();
			}

			QByteArray Aggregator::GetUniqueID () const
			{
				return "org.LeechCraft.Aggregator";
			}

			QString Aggregator::GetName () const
			{
				return "Aggregator";
			}

			QString Aggregator::GetInfo () const
			{
				return tr ("RSS/Atom feed reader.");
			}

			QStringList Aggregator::Provides () const
			{
				return QStringList ("rss");
			}

			QStringList Aggregator::Needs () const
			{
				return QStringList ("http");
			}

			QStringList Aggregator::Uses () const
			{
				return QStringList ("webbrowser");
			}

			void Aggregator::SetProvider (QObject*, const QString&)
			{
			}

			QIcon Aggregator::GetIcon () const
			{
				return QIcon (":/resources/images/aggregator.svg");
			}

			QWidget* Aggregator::GetTabContents ()
			{
				return this;
			}

			QToolBar* Aggregator::GetToolBar () const
			{
				return Impl_->Ui_.ItemsWidget_->GetToolBar ();
			}

			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Aggregator::GetSettingsDialog () const
			{
				return Impl_->XmlSettingsDialog_;
			}

			QAbstractItemModel* Aggregator::GetRepresentation () const
			{
				return Core::Instance ().GetJobHolderRepresentation ();
			}

			void Aggregator::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& index, const QModelIndex&)
			{
				QModelIndex si = Core::Instance ().GetProxy ()->MapToSource (index);
				if (si.model () != GetRepresentation ())
					si = QModelIndex ();
				si = Core::Instance ().GetJobHolderRepresentation ()->SelectionChanged (si);
				Impl_->SelectedRepr_ = si;
				Core::Instance ().GetReprWidget ()->CurrentChannelChanged (si);
			}

			bool Aggregator::CouldHandle (const LeechCraft::Entity& e) const
			{
				return Core::Instance ().CouldHandle (e);
			}

			void Aggregator::Handle (LeechCraft::Entity e)
			{
				Core::Instance ().Handle (e);
			}

#define _LC_MERGE(a) EA##a

#define _LC_SINGLE(a) \
				case Aggregator_Impl::_LC_MERGE(a): \
					Impl_->AppWideActions_.a->setShortcut (shortcut); \
					break;

#define _LC_TRAVERSER(z,i,array) \
				_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))

#define _LC_EXPANDER(Names) \
				switch (name) \
				{ \
					BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names) \
				}
			void Aggregator::SetShortcut (int name,
					const QKeySequence& shortcut)
			{
				_LC_EXPANDER ((ActionAddFeed_)
						(ActionUpdateFeeds_)
						(ActionRegexpMatcher_)
						(ActionImportOPML_)
						(ActionExportOPML_)
						(ActionImportBinary_)
						(ActionExportBinary_)
						(ActionExportFB2_));
			}

#define _L(a) result [Aggregator_Impl::EA##a] = ActionInfo (Impl_->AppWideActions_.a->text (), \
		Impl_->AppWideActions_.a->shortcut (), \
		Impl_->AppWideActions_.a->icon ())
			QMap<int, ActionInfo> Aggregator::GetActionInfo () const
			{
				QMap<int, ActionInfo> result;
				_L (ActionAddFeed_);
				_L (ActionUpdateFeeds_);
				_L (ActionRegexpMatcher_);
				_L (ActionImportOPML_);
				_L (ActionExportOPML_);
				_L (ActionImportBinary_);
				_L (ActionExportBinary_);
				_L (ActionExportFB2_);
				return result;
			}

			QList<QWizardPage*> Aggregator::GetWizardPages () const
			{
				std::auto_ptr<WizardGenerator> wg (new WizardGenerator);
				return wg->GetPages ();
			}

			QList<QAction*> Aggregator::GetActions () const
			{
				QList<QAction*> result;
				result += Impl_->AppWideActions_.ActionAddFeed_;
				result += Impl_->AppWideActions_.ActionUpdateFeeds_;
				return result;
			}

			QList<QAction*> Aggregator::GetTrayActions () const
			{
				QList<QAction*> result;
				result += Impl_->AppWideActions_.ActionAddFeed_;
				result += Impl_->AppWideActions_.ActionUpdateFeeds_;
				return result;
			}

			QList<QMenu*> Aggregator::GetTrayMenus () const
			{
				return QList<QMenu*> ();
			}

			QList<QMenu*> Aggregator::GetToolMenus () const
			{
				QList<QMenu*> result;
				result << Impl_->ToolMenu_;
				return result;
			}

			QList<QAction*> Aggregator::GetToolActions () const
			{
				QList<QAction*> result;
				result << Impl_->AppWideActions_.ActionRegexpMatcher_;
				return result;
			}

			void Aggregator::keyPressEvent (QKeyEvent *e)
			{
				if (e->modifiers () & Qt::ControlModifier)
				{
					QItemSelectionModel *channelSM = Impl_->Ui_.Feeds_->selectionModel ();
					QModelIndex currentChannel = channelSM->currentIndex ();
					int numChannels = Impl_->Ui_.Feeds_->
						model ()->rowCount (currentChannel.parent ());

					QItemSelectionModel::SelectionFlags chanSF =
						QItemSelectionModel::Select |
						QItemSelectionModel::Clear |
						QItemSelectionModel::Rows;

					if (e->key () == Qt::Key_Less &&
							currentChannel.isValid ())
					{
						if (currentChannel.row () > 0)
						{
							QModelIndex next = currentChannel
								.sibling (currentChannel.row () - 1,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						else
						{
							QModelIndex next = currentChannel.sibling (numChannels - 1,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						return;
					}
					else if (e->key () == Qt::Key_Greater &&
							currentChannel.isValid ())
					{
						if (currentChannel.row () < numChannels - 1)
						{
							QModelIndex next = currentChannel
								.sibling (currentChannel.row () + 1,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						else
						{
							QModelIndex next = currentChannel.sibling (0,
											currentChannel.column ());
							channelSM->select (next, chanSF);
							channelSM->setCurrentIndex (next, chanSF);
						}
						return;
					}
					else if ((e->key () == Qt::Key_Greater ||
							e->key () == Qt::Key_Less) &&
							!currentChannel.isValid ())
					{
						QModelIndex next = Impl_->Ui_.Feeds_->model ()->index (0, 0);
						channelSM->select (next, chanSF);
						channelSM->setCurrentIndex (next, chanSF);
					}
				}
				e->ignore ();
			}

			bool Aggregator::IsRepr () const
			{
				return Core::Instance ().GetReprWidget ()->isVisible ();
			}

			QModelIndex Aggregator::GetRelevantIndex () const
			{
				if (IsRepr ())
					return Core::Instance ()
						.GetJobHolderRepresentation ()->
						mapToSource (Impl_->SelectedRepr_);
				else
				{
					QModelIndex index = Impl_->Ui_.Feeds_->
						selectionModel ()->currentIndex ();
					if (Impl_->FlatToFolders_->GetSourceModel ())
						index = Impl_->FlatToFolders_->MapToSource (index);
					return Core::Instance ().GetChannelsModel ()->mapToSource (index);
				}
			}

			void Aggregator::on_ActionAddFeed__triggered ()
			{
				AddFeed af (QString (), this);
				if (af.exec () == QDialog::Accepted)
					Core::Instance ().AddFeed (af.GetURL (), af.GetTags ());
			}

			void Aggregator::on_ActionRemoveFeed__triggered ()
			{
				QModelIndex ds = GetRelevantIndex ();

				if (!ds.isValid ())
					return;

				QString name = ds.sibling (ds.row (), 0).data ().toString ();

				QMessageBox mb (QMessageBox::Warning,
						"LeechCraft",
						tr ("You are going to permanently remove the feed:"
							"<br />%1<br /><br />"
							"Are you really sure that you want to do it?",
							"Feed removing confirmation").arg (name),
						QMessageBox::Ok | QMessageBox::Cancel,
						this);
				mb.setWindowModality (Qt::WindowModal);
				if (mb.exec () == QMessageBox::Ok)
					Core::Instance ().RemoveFeed (ds);
			}

			void Aggregator::on_ActionMarkChannelAsRead__triggered ()
			{
				Core::Instance ().MarkChannelAsRead (GetRelevantIndex ());
			}

			void Aggregator::on_ActionMarkChannelAsUnread__triggered ()
			{
				Core::Instance ().MarkChannelAsUnread (GetRelevantIndex ());
			}

			void Aggregator::on_ActionChannelSettings__triggered ()
			{
				QModelIndex index = GetRelevantIndex ();
				if (!index.isValid ())
					return;

				std::auto_ptr<FeedSettings> dia (new FeedSettings (index, this));
				dia->exec ();
			}

			void Aggregator::on_ActionUpdateSelectedFeed__triggered ()
			{
				bool isRepr = IsRepr ();
				QModelIndex current = GetRelevantIndex ();

				if (!current.isValid ())
				{
					qWarning () << Q_FUNC_INFO
						<< current
						<< isRepr;
					return;
				}
				Core::Instance ().UpdateFeed (current, isRepr);
			}

			void Aggregator::on_ActionRegexpMatcher__triggered ()
			{
				Impl_->RegexpMatcherUi_->show ();
			}

			void Aggregator::on_ActionImportOPML__triggered ()
			{
				Core::Instance ().StartAddingOPML (QString ());
			}

			void Aggregator::on_ActionExportOPML__triggered ()
			{
				Export exportDialog (tr ("Export to OPML"),
						tr ("Select save file"),
						tr ("OPML files (*.opml);;"
							"XML files (*.xml);;"
							"All files (*.*)"), this);
				channels_shorts_t channels;
				Core::Instance ().GetChannels (channels);
				exportDialog.SetFeeds (channels);
				if (exportDialog.exec () == QDialog::Rejected)
					return;

				Core::Instance ().ExportToOPML (exportDialog.GetDestination (),
						exportDialog.GetTitle (),
						exportDialog.GetOwner (),
						exportDialog.GetOwnerEmail (),
						exportDialog.GetSelectedFeeds ());
			}

			void Aggregator::on_ActionImportBinary__triggered ()
			{
				ImportBinary import (this);
				if (import.exec () == QDialog::Rejected)
					return;

				Core::Instance ().AddFeeds (import.GetSelectedFeeds (),
						import.GetTags ());
			}

			void Aggregator::on_ActionExportBinary__triggered ()
			{
				Export exportDialog (tr ("Export to binary file"),
						tr ("Select save file"),
						tr ("Aggregator exchange files (*.lcae);;"
							"All files (*.*)"), this);
				channels_shorts_t channels;
				Core::Instance ().GetChannels (channels);
				exportDialog.SetFeeds (channels);
				if (exportDialog.exec () == QDialog::Rejected)
					return;

				Core::Instance ().ExportToBinary (exportDialog.GetDestination (),
						exportDialog.GetTitle (),
						exportDialog.GetOwner (),
						exportDialog.GetOwnerEmail (),
						exportDialog.GetSelectedFeeds ());
			}

			void Aggregator::on_ActionExportFB2__triggered ()
			{
				Export2FB2Dialog *dialog = new Export2FB2Dialog (this);
				dialog->setAttribute (Qt::WA_DeleteOnClose);
				dialog->show ();
			}

			void Aggregator::on_MergeItems__toggled (bool merge)
			{
				Impl_->Ui_.ItemsWidget_->SetMergeMode (merge);
				XmlSettingsManager::Instance ()->setProperty ("MergeItems", merge);
			}

			void Aggregator::currentChannelChanged ()
			{
				QModelIndex index = Impl_->Ui_.Feeds_->
						selectionModel ()->currentIndex ();
				if (Impl_->FlatToFolders_->GetSourceModel ())
				{
					QModelIndex origIndex = index;
					index = Impl_->FlatToFolders_->MapToSource (index);
					if (!index.isValid ())
					{
						QStringList tags = origIndex.data (RoleTags).toStringList ();
						Impl_->Ui_.ItemsWidget_->SetMergeModeTags (tags);
						return;
					}
				}
				Impl_->Ui_.ItemsWidget_->CurrentChannelChanged (index);
			}

			void Aggregator::unreadNumberChanged (int number)
			{
				if (!number ||
						!XmlSettingsManager::Instance ()->
							property ("ShowIconInTray").toBool ())
				{
					Impl_->TrayIcon_->hide ();
					return;
				}

				QString tip = tr ("%n unread message(s)", "", number) + " " +
					      tr ("in %n channel(s).", "", Core::Instance ().GetUnreadChannelsNumber ());
				Impl_->TrayIcon_->setToolTip (tip);
				Impl_->TrayIcon_->show ();
			}

			void Aggregator::trayIconActivated ()
			{
				emit bringToFront ();
				QModelIndex unread = Core::Instance ().GetUnreadChannelIndex ();
				if (unread.isValid ())
				{
					if (Impl_->FlatToFolders_->GetSourceModel ())
						unread = Impl_->FlatToFolders_->MapFromSource (unread).at (0);
					Impl_->Ui_.Feeds_->setCurrentIndex (unread);
				}
			}

			void Aggregator::handleGroupChannels ()
			{
				if (XmlSettingsManager::Instance ()->
						property ("GroupChannelsByTags").toBool ())
				{
					Impl_->FlatToFolders_->SetSourceModel (Core::Instance ().GetChannelsModel ());
					Impl_->Ui_.Feeds_->setModel (Impl_->FlatToFolders_.get ());
				}
				else
				{
					Impl_->FlatToFolders_->SetSourceModel (0);
					Impl_->Ui_.Feeds_->setModel (Core::Instance ().GetChannelsModel ());
				}
				connect (Impl_->Ui_.Feeds_->selectionModel (),
						SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
						this,
						SLOT (currentChannelChanged ()));
				Impl_->Ui_.Feeds_->expandAll ();
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_aggregator, LeechCraft::Plugins::Aggregator::Aggregator);

