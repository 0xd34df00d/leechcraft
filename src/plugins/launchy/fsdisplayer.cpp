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

#include "fsdisplayer.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QStandardItemModel>
#include "itemsfinder.h"
#include "item.h"

namespace LeechCraft
{
namespace Launchy
{
	namespace
	{
		class DisplayModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				CategoryName,
				CategoryIcon
			};

			DisplayModel (QObject *parent = 0)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [CategoryName] = "categoryName";
				roleNames [CategoryIcon] = "categoryIcon";
				setRoleNames (roleNames);
			}
		};
	}

	FSDisplayer::FSDisplayer (ItemsFinder *finder, QObject *parent)
	: QObject (parent)
	, Finder_ (finder)
	, Model_ (new DisplayModel (this))
	, View_ (new QDeclarativeView)
	{
		View_->setStyleSheet ("background: transparent");
		View_->setWindowFlags (Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		View_->setAttribute (Qt::WA_TranslucentBackground);
		View_->setAttribute (Qt::WA_OpaquePaintEvent, false);
		View_->setFixedSize (QSize (800, 600));

		View_->setResizeMode (QDeclarativeView::SizeRootObjectToView);
		View_->rootContext ()->setContextProperty ("itemsModel", Model_);
		View_->setSource (QUrl ("qrc:/launchy/resources/qml/FSView.qml"));

		View_->show ();

		View_->setFocus ();
		View_->setFocus (Qt::MouseFocusReason);

		connect (View_->rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));

		handleFinderUpdated ();
	}

	FSDisplayer::~FSDisplayer ()
	{
		delete View_;
	}

	void FSDisplayer::handleFinderUpdated ()
	{
		Model_->clear ();

		const auto& items = Finder_->GetItems ();
		for (const auto& cat : items.keys ())
		{
			auto catItem = new QStandardItem (cat);
			catItem->setData (cat, DisplayModel::Roles::CategoryName);

			for (const auto& item : items [cat])
			{
				auto appItem = new QStandardItem ();
				catItem->appendRow (appItem);
			}

			Model_->appendRow (catItem);
		}
	}
}
}
