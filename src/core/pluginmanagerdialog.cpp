/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pluginmanagerdialog.h"
#include <QStyledItemDelegate>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include "util/gui/clearlineeditaddon.h"
#include "interfaces/ihavesettings.h"
#include "interfaces/iinfo.h"
#include "core.h"
#include "iconthemeengine.h"
#include "coreinstanceobject.h"
#include "settingstab.h"
#include "coreproxy.h"

namespace LC
{
	namespace
	{
		class PrefDelegate : public QStyledItemDelegate
		{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			QWidget* createEditor (QWidget *parent,
					const QStyleOptionViewItem& option,
					const QModelIndex& index) const override
			{
				if (index.column () != 2 ||
						!(index.flags () & Qt::ItemIsEditable))
					return QStyledItemDelegate::createEditor (parent, option, index);

				QPushButton *button = new QPushButton (parent);
				button->setIcon (IconThemeEngine::Instance ().GetIcon ("configure", QString ()));
				button->setToolTip (tr ("Configure..."));
				button->setMaximumWidth (48);

				const auto pluginObj = index.data (PluginManager::Roles::PluginObject).value<QObject*> ();
				connect (button,
						&QPushButton::released,
						[pluginObj]
						{
							Core::Instance ().GetCoreInstanceObject ()->GetSettingsTab ()->showSettingsFor (pluginObj);
						});
				return button;
			}

			void updateEditorGeometry (QWidget *editor,
					const QStyleOptionViewItem& option,
					const QModelIndex&) const override
			{
				editor->setGeometry (option.rect);
			}

			QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				return index.column () == 2 ?
						QSize (32, 32) :
						QStyledItemDelegate::sizeHint (option, index);
			}
		};

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

		class PluginsProxyModel : public QSortFilterProxyModel
		{
		public:
			PluginsProxyModel (QObject *parent = 0)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex&) const
			{
				const QString& filter = filterRegExp ().pattern ();
				if (filter.isEmpty ())
					return true;

				auto m = sourceModel ();
				for (int c = 0, ccount = m->columnCount (); c < ccount; ++c)
					if (m->index (row, c).data ().toString ().contains (filter, Qt::CaseInsensitive))
						return true;

				return false;
			}

			bool lessThan (const QModelIndex& left, const QModelIndex& right) const
			{
				const QString& lPath = left.data (PluginManager::Roles::PluginFilename).toString ();
				const QString& rPath = right.data (PluginManager::Roles::PluginFilename).toString ();
				if (!lPath.isEmpty () && !rPath.isEmpty ())
					return lPath < rPath;
				return QSortFilterProxyModel::lessThan (left, right);
			}
		};
	}

	PluginManagerDialog::PluginManagerDialog (QWidget *parent)
	: QWidget (parent)
	, FilterProxy_ (new PluginsProxyModel (this))
	{
		Ui_.setupUi (this);
		Ui_.PluginsTree_->setWordWrap (true);

		auto model = Core::Instance ().GetPluginsModel ();
		FilterProxy_->setSourceModel (Core::Instance ().GetPluginsModel ());
		Ui_.PluginsTree_->setModel (FilterProxy_);
		Ui_.PluginsTree_->setItemDelegateForColumn (2, new PrefDelegate (this));

		for (int i = 0, rc = model->rowCount (); i < rc; ++i)
		{
			const auto& idx = model->index (i, 2);
			if (idx.flags () & Qt::ItemIsEditable)
				Ui_.PluginsTree_->openPersistentEditor (FilterProxy_->mapFromSource (idx));
		}

		Ui_.PluginsTree_->installEventFilter (new SizeFilter (this));

		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				FilterProxy_,
				SLOT (setFilterFixedString (QString)));

		new Util::ClearLineEditAddon (CoreProxy::UnsafeWithoutDeps (), Ui_.FilterLine_);
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
