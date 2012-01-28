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

#include "pluginmanagerdialog.h"
#include <QStyledItemDelegate>
#include <QPushButton>
#include "interfaces/ihavesettings.h"
#include "interfaces/iinfo.h"
#include "core.h"
#include "skinengine.h"
#include "coreinstanceobject.h"
#include "settingstab.h"

namespace LeechCraft
{
	class PrefDelegate : public QStyledItemDelegate
	{
	public:
		PrefDelegate (QObject* parent = 0)
		: QStyledItemDelegate (parent)
		{
		}

		virtual QWidget* createEditor (QWidget *parent,
				const QStyleOptionViewItem& option,
				const QModelIndex& index) const
		{
			if (index.column () != 2 ||
					!(index.flags () & Qt::ItemIsEditable))
				return QStyledItemDelegate::createEditor (parent, option, index);

			QPushButton *button = new QPushButton (parent);
			button->setIcon (SkinEngine::Instance ().GetIcon ("configure", QString ()));
			button->setToolTip (tr ("Configure..."));
			button->setMaximumWidth (48);
			button->setProperty ("SettableObject",
					index.data (PluginManager::Roles::PluginObject));
			connect (button,
					SIGNAL (released ()),
					Core::Instance ().GetCoreInstanceObject ()->GetSettingsTab (),
					SLOT (handleSettingsCalled ()));
			return button;
		}

		void updateEditorGeometry (QWidget *editor,
				const QStyleOptionViewItem& option,
				const QModelIndex&) const
		{
			editor->setGeometry (option.rect);
		}

		QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			return index.column () == 2 ?
					QSize (32, 32) :
					QStyledItemDelegate::sizeHint (option, index);
		}
	};

	namespace
	{
		class SizeFilter : public QObject
		{
			PluginManagerDialog *PMD_;
		public:
			SizeFilter (PluginManagerDialog *pmd)
			: QObject (pmd)
			, PMD_ (pmd)
			{
			}

			bool eventFilter (QObject *obj, QEvent *e)
			{
				if (e->type () == QEvent::Show ||
						e->type () == QEvent::Resize)
					PMD_->readjustColumns ();

				return QObject::eventFilter (obj, e);
			}
		};
	}

	PluginManagerDialog::PluginManagerDialog (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Ui_.PluginsTree_->setWordWrap (true);

		auto model = Core::Instance ().GetPluginsModel ();
		Ui_.PluginsTree_->setModel (model);
		Ui_.PluginsTree_->setItemDelegateForColumn (2, new PrefDelegate (this));

		for (int i = 0, rc = model->rowCount (); i < rc; ++i)
		{
			const auto& idx = model->index (i, 2);
			if (idx.flags () & Qt::ItemIsEditable)
				Ui_.PluginsTree_->openPersistentEditor (idx);
		}

		Ui_.PluginsTree_->installEventFilter (new SizeFilter (this));
	}

	void PluginManagerDialog::readjustColumns ()
	{
		Ui_.PluginsTree_->setColumnWidth (1,
				Ui_.PluginsTree_->viewport ()->width () - 48 - Ui_.PluginsTree_->columnWidth (0));
		Ui_.PluginsTree_->setColumnWidth (2, 48);
	}

	void PluginManagerDialog::accept ()
	{
	}

	void PluginManagerDialog::reject ()
	{
	}
}
