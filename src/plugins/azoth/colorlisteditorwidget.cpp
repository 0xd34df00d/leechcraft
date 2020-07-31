/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colorlisteditorwidget.h"
#include <QStandardItemModel>
#include <QColorDialog>
#include <QPainter>
#include <QtDebug>
#include "xmlsettingsmanager.h"

Q_DECLARE_METATYPE (QList<QColor>);

namespace LC
{
namespace Azoth
{
	namespace
	{
		enum ColorListRoles
		{
			Color = Qt::UserRole + 1
		};
	}

	ColorListEditorWidget::ColorListEditorWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ColorsView_->setModel (Model_);

		reject ();
	}

	void ColorListEditorWidget::AddColor (const QColor& color)
	{
		QPixmap px (32, 32);
		QPainter p (&px);
		p.fillRect (px.rect (), color);
		p.end ();

		auto item = new QStandardItem;
		item->setText (color.name ());
		item->setIcon (px);
		item->setForeground (color);
		item->setData (color, ColorListRoles::Color);
		Model_->appendRow (item);
	}

	void ColorListEditorWidget::accept ()
	{
		QList<QColor> colors;
		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
		{
			auto item = Model_->item (i);
			colors << item->data (ColorListRoles::Color).value<QColor> ();
		}

		XmlSettingsManager::Instance ().setProperty ("OverrideColorsList",
				QVariant::fromValue (colors));
	}

	void ColorListEditorWidget::reject ()
	{
		Model_->clear ();
		const auto& var = XmlSettingsManager::Instance ()
				.property ("OverrideColorsList");
		const auto& colors = var.value<QList<QColor>> ();
		for (const auto& color : colors)
			AddColor (color);
	}

	void ColorListEditorWidget::on_AddColorButton__released ()
	{
		QColorDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		AddColor (dia.selectedColor ());
	}

	void ColorListEditorWidget::on_RemoveColorButton__released ()
	{
		const auto& index = Ui_.ColorsView_->currentIndex ();
		if (!index.isValid ())
			return;

		Model_->removeRow (index.row ());
	}
}
}
