/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "formmanager.h"
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QtDebug>
#include "interfaces/monocle/isupportforms.h"
#include "interfaces/monocle/iformfield.h"
#include "pagegraphicsitem.h"

namespace LeechCraft
{
namespace Monocle
{
	FormManager::FormManager (QGraphicsView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Scene_ (view->scene ())
	{
	}

	void FormManager::HandleDoc (IDocument_ptr doc, const QList<PageGraphicsItem*>& pages)
	{
		auto formsDoc = dynamic_cast<ISupportForms*> (doc.get ());
		if (!formsDoc)
			return;

		for (auto page : pages)
			for (auto field : formsDoc->GetFormFields (page->GetPageNum ()))
				switch (field->GetType ())
				{
				case FormType::Text:
					AddTextField (field, page);
					break;
				}
	}

	void FormManager::AddTextField (std::shared_ptr<IFormField> baseField, PageGraphicsItem *page)
	{
		const auto field = std::dynamic_pointer_cast<IFormFieldText> (baseField);
		QGraphicsProxyWidget *proxy = 0;
		switch (field->GetTextType ())
		{
		case IFormFieldText::Type::SingleLine:
		{
			auto edit = new QLineEdit ();
			edit->setText (field->GetText ());
			if (field->IsPassword ())
				edit->setEchoMode (QLineEdit::Password);
			if (field->GetMaximumLength () > 0)
				edit->setMaxLength (field->GetMaximumLength ());
			proxy = Scene_->addWidget (edit);
			break;
		}
		case IFormFieldText::Type::Multiline:
		{
			auto edit = new QTextEdit ();
			edit->setText (field->GetText ());
			edit->setAcceptRichText (field->IsRichText ());
			proxy = Scene_->addWidget (edit);
			break;
		}
		default:
			qWarning () << Q_FUNC_INFO
					<< "unsupported type";
			break;
		}

		if (!proxy)
			return;

		const auto& docRect = page->MapToDoc (page->boundingRect ());

		const auto& formRect = baseField->GetRect ();

		QRectF targetRect (formRect.x () * docRect.width (),
				formRect.y () * docRect.height (),
				formRect.width () * docRect.width (),
				formRect.height () * docRect.height ());

		proxy->setParentItem (page);
		page->RegisterChildRect (proxy, targetRect,
				[proxy] (const QRectF& rect) { proxy->setGeometry (rect); });
	}
}
}
