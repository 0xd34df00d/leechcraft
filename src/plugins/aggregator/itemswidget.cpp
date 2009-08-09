#include "itemswidget.h"
#include <memory>
#include <QFileInfo>
#include <QHeaderView>
#include <QtDebug>
#include <QUrl>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/categoryselector.h>
#include <plugininterface/proxy.h>
#include "core.h"
#include "itemsfiltermodel.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			using LeechCraft::Util::CategorySelector;
			
			struct ItemsWidget_Impl
			{
				Ui::ItemsWidget Ui_;

				QAction *ActionMarkItemAsUnread_;
				QAction *ActionAddToItemBucket_;
				QAction *ActionItemCommentsSubscribe_;

				bool TapeMode_;

				std::auto_ptr<ItemsFilterModel> ItemsFilterModel_;
				std::auto_ptr<CategorySelector> ItemCategorySelector_;
			};
			
			ItemsWidget::ItemsWidget (QWidget *parent)
			: QWidget (parent)
			{
				Impl_ = new ItemsWidget_Impl;
				Impl_->TapeMode_ = false;
				Impl_->ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"),
						this);
				Impl_->ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");
			
				Impl_->ActionAddToItemBucket_ = new QAction (tr ("Add to item bucket"),
						this);
				Impl_->ActionAddToItemBucket_->setObjectName ("ActionAddToItemBucket_");

				Impl_->ActionItemCommentsSubscribe_ = new QAction (tr ("Subscribe to comments"),
						this);
				Impl_->ActionItemCommentsSubscribe_->setObjectName ("ActionItemCommentsSubscribe_");
			
				Impl_->Ui_.setupUi (this);
				Impl_->Ui_.ItemView_->Construct (Core::Instance ().GetWebBrowser ());
			
				Impl_->Ui_.Items_->setAcceptDrops (false);
			
				Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);
				Impl_->ItemsFilterModel_.reset (new ItemsFilterModel (this));
				Impl_->ItemsFilterModel_->setSourceModel (Core::Instance ().GetItemsModel ());
				connect (Core::Instance ().GetItemsModel (),
						SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
						Impl_->ItemsFilterModel_.get (),
						SLOT (invalidate ()));
				Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
				Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
				Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());
			
				Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsUnread_);
				Impl_->Ui_.Items_->addAction (Impl_->ActionAddToItemBucket_);
				Impl_->Ui_.Items_->addAction (Impl_->ActionItemCommentsSubscribe_);
				Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
				connect (Impl_->Ui_.SearchLine_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (updateItemsFilter ()));
				connect (Impl_->Ui_.SearchType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (updateItemsFilter ()));
			
				connect (&Core::Instance (),
						SIGNAL (currentChannelChanged (const QModelIndex&)),
						this,
						SLOT (channelChanged (const QModelIndex&)));
			
				QHeaderView *itemsHeader = Impl_->Ui_.Items_->header ();
				QFontMetrics fm = fontMetrics ();
				int dateTimeSize = fm.width (QDateTime::currentDateTime ()
						.toString (Qt::SystemLocaleShortDate) + "__");
				itemsHeader->resizeSection (0,
						fm.width ("Average news article size is about this width or "
							"maybe bigger, because they are bigger"));
				itemsHeader->resizeSection (1,
						dateTimeSize);
				connect (Impl_->Ui_.Items_->header (),
						SIGNAL (sectionClicked (int)),
						this,
						SLOT (makeCurrentItemVisible ()));
			
				Impl_->ItemCategorySelector_.reset (new CategorySelector ());
				Impl_->ItemCategorySelector_->setWindowFlags (Qt::Widget);
				Impl_->Ui_.CategoriesLayout_->addWidget (Impl_->ItemCategorySelector_.get ());
				Impl_->ItemCategorySelector_->hide ();
				Impl_->ItemCategorySelector_->setMinimumHeight (0);
				connect (Impl_->ItemCategorySelector_.get (),
						SIGNAL (selectionChanged (const QStringList&)),
						Impl_->ItemsFilterModel_.get (),
						SLOT (categorySelectionChanged (const QStringList&)));
			
				connect (Impl_->Ui_.Items_->selectionModel (),
						SIGNAL (selectionChanged (const QItemSelection&,
								const QItemSelection&)),
						this,
						SLOT (currentItemChanged (const QItemSelection&)),
						Qt::QueuedConnection);
			
				currentItemChanged (QItemSelection ());

				XmlSettingsManager::Instance ()->RegisterObject ("ShowCategorySelector",
						this, "selectorVisiblityChanged");
			}
			
			ItemsWidget::~ItemsWidget ()
			{
				disconnect (Impl_->ItemsFilterModel_.get (),
						0,
						this,
						0);
				disconnect (Impl_->ItemCategorySelector_.get (),
						0,
						this,
						0);
				delete Impl_;
			}
			
			void ItemsWidget::SetTapeMode (bool tape)
			{
				Impl_->TapeMode_ = tape;
				if (tape)
					disconnect (Impl_->Ui_.Items_->selectionModel (),
							SIGNAL (selectionChanged (const QItemSelection&,
									const QItemSelection&)),
							this,
							SLOT (currentItemChanged (const QItemSelection&)));
				else
					connect (Impl_->Ui_.Items_->selectionModel (),
							SIGNAL (selectionChanged (const QItemSelection&,
									const QItemSelection&)),
							this,
							SLOT (currentItemChanged (const QItemSelection&)),
							Qt::QueuedConnection);
				currentItemChanged (QItemSelection ());
			}
			
			void ItemsWidget::SetHideRead (bool hide)
			{
				Impl_->ItemsFilterModel_->SetHideRead (hide);
			}
			
			QString ItemsWidget::GetHex (QPalette::ColorRole role, QPalette::ColorGroup group)
			{
				int r, g, b;
				QApplication::palette ().color (group, role).getRgb (&r, &g, &b);
				long color = b + (g << 8) + (b << 16);
				QString result ("#%1");
				// Fill spare space with zeros.
				return result.arg (color, 6, 16, QChar ('0'));
			}
			
			QString ItemsWidget::ToHtml (const Item_ptr& item)
			{
				QPalette palette = QApplication::palette ();
			
				QString headerBg = GetHex (QPalette::Window);
				QString headerText = GetHex (QPalette::WindowText);
			
				QString startBox = "<div style='background: %1; "
					"color: COLOR; "
					"margin-left: -1em;"
					"margin-right: -1em; "
					"padding-left: 2em; "
					"padding-right: 2em;'>";
				startBox.replace ("COLOR", headerText);
			
				bool linw = XmlSettingsManager::Instance ()->
						property ("AlwaysUseExternalBrowser").toBool ();
			
				QString result = QString ("<div style='background: %1; "
						"margin-left: 0px; "
						"margin-right: 0px; "
						"padding-left: 1em; "
						"padding-right: 1em'>")
					.arg (GetHex (QPalette::Base));
			
				// Title
				result += startBox.arg (headerBg);
				result += (QString ("<strong>") +
						item->Title_ +
						"</strong></div>");
			
				// Link
				result += (startBox.arg (headerBg) +
						"<a href='" +
						item->Link_ +
						"'");
				if (linw)
					result += " target='_blank'";
				result += (QString (">") +
						item->Link_ +
						"</a></div>");
			
				// Publication date and author
				if (item->PubDate_.isValid () && !item->Author_.isEmpty ())
					result += (startBox.arg (headerBg) +
							tr ("Published on %1 by %2")
						   		.arg (item->PubDate_.toString ())
								.arg (item->Author_) +
							"</div>");
				else if (item->PubDate_.isValid ())
					result += (startBox.arg (headerBg) +
							tr ("Published on %1")
						   		.arg (item->PubDate_.toString ()) +
							"</div>");
				else if (!item->Author_.isEmpty ())
					result += (startBox.arg (headerBg) +
							tr ("Published by %1")
								.arg (item->Author_) +
							"</div>");
			
				// Categories
				if (item->Categories_.size ())
					result += (startBox.arg (headerBg) +
							item->Categories_.join ("; ") +
							"</div>");
			
				// Comments stuff
				if (item->NumComments_ >= 0 && !item->CommentsPageLink_.isEmpty ())
					result += (startBox.arg (headerBg) + 
							tr ("%1 comments, <a href='%2'%3>view them</a></div>")
								.arg (item->NumComments_)
								.arg (item->CommentsPageLink_)
								.arg (linw ? " target='_blank'" : ""));
				else if (item->NumComments_ >= 0)
					result += (startBox.arg (headerBg) + 
							tr ("%1 comments</div>")
								.arg (item->NumComments_));
				else if (!item->CommentsPageLink_.isEmpty ())
					result += (startBox.arg (headerBg) + 
							tr ("<a href='%1'%2>View comments</a></div>")
								.arg (item->CommentsPageLink_)
								.arg (linw ? " target='_blank'" : ""));
			
				result += "<br />";
				result += QString ("<div style='color: %2'>")
					.arg (GetHex (QPalette::Text));
			
				// Description
				result += item->Description_;
				for (QList<Enclosure>::const_iterator i = item->Enclosures_.begin (),
						end = item->Enclosures_.end (); i != end; ++i)
				{
					result += QString ("<div style='background: %1; "
						"color: %2; "
						"border: 1px solid #333333; "
						"padding-top: 1em; "
						"padding-bottom: 1em; "
						"padding-left: 2em; "
						"padding-right: 2em;'>")
						.arg (headerBg)
						.arg (headerText);
					if (i->Length_ > 0)
						result += tr ("File of type %1, size %2:<br />")
							.arg (i->Type_)
							.arg (LeechCraft::Util::Proxy::Instance ()->
									MakePrettySize (i->Length_));
					else
						result += tr ("File of type %1 and unknown length:<br />")
							.arg (i->Type_);
					result += QString ("<a href='%1'>%2</a>")
						.arg (i->URL_)
						.arg (QFileInfo (QUrl (i->URL_).path ()).fileName ());
					if (!i->Lang_.isEmpty ())
						result += tr ("<br />Specified language: %1")
							.arg (i->Lang_);
					result += "</div>";
				}
			
				result += "</div>";
				result += "</div>";
			
				return result;
			}
			
			void ItemsWidget::channelChanged (const QModelIndex& mapped)
			{
				Impl_->Ui_.Items_->scrollToTop ();
				currentItemChanged (QItemSelection ());
				
				if (!isVisible ())
					return;
			
				QStringList allCategories = Core::Instance ().GetCategories (mapped);
				Impl_->ItemsFilterModel_->categorySelectionChanged (allCategories);

				if (allCategories.size ())
				{
					Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
					Impl_->ItemCategorySelector_->selectAll ();
					if (XmlSettingsManager::Instance ()->
							property ("ShowCategorySelector").toBool ())
					Impl_->ItemCategorySelector_->show ();
				}
				else
				{
					Impl_->ItemCategorySelector_->SetPossibleSelections (QStringList ());
					Impl_->ItemCategorySelector_->hide ();
				}
			}
			
			void ItemsWidget::on_ActionMarkItemAsUnread__triggered ()
			{
				QModelIndexList indexes = Impl_->Ui_.Items_->
					selectionModel ()->selectedRows ();
				for (int i = 0; i < indexes.size (); ++i)
					Core::Instance ().MarkItemAsUnread (Impl_->
							ItemsFilterModel_->mapToSource (indexes.at (i)));
			}
			
			void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
			{
				Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
						Qt::CaseSensitive : Qt::CaseInsensitive);
			}
			
			void ItemsWidget::on_ActionAddToItemBucket__triggered ()
			{
				Core::Instance ().AddToItemBucket (Impl_->ItemsFilterModel_->
						mapToSource (Impl_->Ui_.Items_->selectionModel ()->
							currentIndex ()));
			}
			
			void ItemsWidget::on_ActionItemCommentsSubscribe__triggered ()
			{
				QModelIndex selected = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
				Core::Instance ().SubscribeToComments (Impl_->ItemsFilterModel_->
						mapToSource (selected));
			}
			
			void ItemsWidget::currentItemChanged (const QItemSelection& selection)
			{
				if (Impl_->TapeMode_)
				{
					QString html;
					for (int i = 0, size = Impl_->ItemsFilterModel_->rowCount ();
							i < size; ++i)
					{
						QModelIndex index = Impl_->ItemsFilterModel_->index (i, 0);
						QModelIndex mapped = Impl_->ItemsFilterModel_->mapToSource (index);
						Item_ptr item = Core::Instance ().GetItem (mapped);
			
						html += ToHtml (item);
						html += "<hr />";
					}
			
					Impl_->Ui_.ItemView_->SetHtml (html);
				}
				else
				{
					QModelIndexList indexes = selection.indexes ();
			
					QModelIndex sindex;
					if (indexes.size ())
						sindex = Impl_->ItemsFilterModel_->mapToSource (indexes.at (0));
			
					if (!sindex.isValid () || indexes.size () != 2)
					{
						Impl_->Ui_.ItemView_->SetHtml ("");
						Impl_->ActionItemCommentsSubscribe_->setEnabled (false);
						return;
					}
			
					Core::Instance ().Selected (sindex);
			
					Item_ptr item = Core::Instance ().GetItem (sindex);
			
					Impl_->Ui_.ItemView_->SetHtml (ToHtml (item));
			
					QString commentsRSS = item->CommentsLink_;
					Impl_->ActionItemCommentsSubscribe_->setEnabled (!commentsRSS.isEmpty ());
				}
			}
			
			void ItemsWidget::makeCurrentItemVisible ()
			{
				QModelIndex item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
				if (item.isValid ())
					Impl_->Ui_.Items_->scrollTo (item);
			}
			
			void ItemsWidget::updateItemsFilter ()
			{
				int section = Impl_->Ui_.SearchType_->currentIndex ();
				QString text = Impl_->Ui_.SearchLine_->text ();
				switch (section)
				{
				case 1:
					Impl_->ItemsFilterModel_->setFilterWildcard (text);
					break;
				case 2:
					Impl_->ItemsFilterModel_->setFilterRegExp (text);
					break;
				default:
					Impl_->ItemsFilterModel_->setFilterFixedString (text);
					break;
				}
			}

			void ItemsWidget::selectorVisiblityChanged ()
			{
				if (!XmlSettingsManager::Instance ()->
						property ("ShowCategorySelector").toBool ())
				{
					Impl_->ItemCategorySelector_->selectAll ();
					Impl_->ItemCategorySelector_->hide ();
				}
				else if (Impl_->ItemCategorySelector_->GetSelections ().size ())
					Impl_->ItemCategorySelector_->show ();
			}
		};
	};
};

