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

#include "startupthirdpage.h"
#include <QLineEdit>
#include <QTextCodec>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			StartupThirdPage::FeedInfo::FeedInfo (const QString& name,
					const QString& tags, const QString& url)
			: Name_ (name)
			, DefaultTags_ (tags)
			, URL_ (url)
			{
			}

			StartupThirdPage::StartupThirdPage (QWidget *parent)
			: QWizardPage (parent)
			{
				Sets_ ["general"] << FeedInfo (QString::fromUtf8 ("Slashdot"),
						"it; news; software; science; world",
						"http://rss.slashdot.org/Slashdot/slashdot");
				Sets_ ["general"] << FeedInfo (QString::fromUtf8 ("Qt Labs Blogs"),
						"it; qt; news; blogs; programming",
						"http://labs.trolltech.com/blogs/feed/atom/");
				Sets_ ["general"] << FeedInfo (QString::fromUtf8 ("euronews RSS feed"),
						"news; world",
						"http://feeds.feedburner.com/euronews/en/home/");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Linux.org.ru: Новости"),
						"it; news; software",
						"http://www.linux.org.ru/section-rss.jsp?section=1");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("OpenNews.opennet.ru: Основная лента"),
						"it; news; software",
						"http://www.opennet.ru/opennews/opennews_6.rss");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Хабрахабр"),
						"it; news; blogs; software; science",
						"http://habrahabr.ru/rss/");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Радио-Т"),
						"it; blogs; podcasts",
						"http://feeds.rucast.net/radio-t");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Новости: Hardware на iXBT.com"),
						"it; news; hardware",
						"http://www.ixbt.com/export/hardnews.rss");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Новости: Software на iXBT.com"),
						"it; news; software",
						"http://www.ixbt.com/export/softnews.rss");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Статьи на iXBT.com"),
						"it",
						"http://www.ixbt.com/export/articles.rss");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("3Dnews - Новости Hardware"),
						"it; news; hardware",
						"http://www.3dnews.ru/news/rss/");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("MEMBRANA: Люди. Идеи. Технологии."),
						"news; science; world",
						"http://www.membrana.ru/export/rss.xml");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Яндекс.Новости"),
						"news; world",
						"http://news.yandex.ru/index.rss");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Lenta.Ru"),
						"news",
						"http://lenta.ru/rss/");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("Bash.Org.Ru"),
						"fun",
						"http://bash.org.ru/rss/");
				Sets_ ["ru"] << FeedInfo (QString::fromUtf8 ("iBash.Org.Ru"),
						"fun; it",
						"http://ibash.org.ru/rss.xml");

				Ui_.setupUi (this);
				Ui_.Tree_->header ()->setResizeMode (0, QHeaderView::ResizeToContents);
				Ui_.Tree_->header ()->setResizeMode (1, QHeaderView::ResizeToContents);

				connect (Ui_.LocalizationBox_,
						SIGNAL (currentIndexChanged (const QString&)),
						this,
						SLOT (handleCurrentIndexChanged (const QString&)));

				QMap<QString, int> languages;
				languages ["ru"] = 1;

				QString language = Util::GetLanguage ();
				Ui_.LocalizationBox_->setCurrentIndex (languages.contains (language) ?
						languages [language] :
						0);
				handleCurrentIndexChanged (QString ("(") + language + ")");

				setTitle ("Aggregator");
				setSubTitle (tr ("Select feeds"));
			}

			void StartupThirdPage::initializePage ()
			{
				connect (wizard (),
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));
				wizard ()->setMinimumWidth (std::max (wizard ()->minimumWidth (), 800));
				wizard ()->setMinimumHeight (std::max (wizard ()->minimumHeight (), 500));

				XmlSettingsManager::Instance ()->
						setProperty ("StartupVersion", 3);
			}

			void StartupThirdPage::Populate (const QString& title)
			{
				FeedInfos_t engines = Sets_ [title];
				Q_FOREACH (FeedInfo info, engines)
				{
					QStringList strings;
					strings << info.Name_
						<< info.DefaultTags_
						<< info.URL_;

					QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.Tree_, strings);
					item->setCheckState (0, Qt::Checked);

					QLineEdit *edit = new QLineEdit (Ui_.Tree_);
					edit->setFrame (false);
					edit->setText (info.DefaultTags_);
					Ui_.Tree_->setItemWidget (item, 1, edit);
				}
			}

			void StartupThirdPage::handleAccepted ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
				{
					QTreeWidgetItem *item = Ui_.Tree_->topLevelItem (i);
					if (item->checkState (0) != Qt::Checked)
						continue;

					QString url = item->text (2);
					QString tags = static_cast<QLineEdit*> (Ui_.Tree_->itemWidget (item, 1))->text ();
					Core::Instance ().AddFeed (url, tags);
				}
			}

			void StartupThirdPage::handleCurrentIndexChanged (const QString& text)
			{
				Ui_.Tree_->clear ();
				if (text.endsWith (')'))
				{
					QString selected = text.mid (text.size () - 3, 2);
					Populate (selected);
				}
				Populate ("general");
			}

			void StartupThirdPage::on_SelectAll__released ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Checked);
			}

			void StartupThirdPage::on_DeselectAll__released ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Unchecked);
			}
		};
	};
};

