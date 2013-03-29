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

#pragma once

#include <QObject>
#include <QGraphicsProxyWidget>
#include "interfaces/monocle/idocument.h"

class QGraphicsView;
class QGraphicsScene;
class QLineEdit;
class QTextEdit;
class QComboBox;

namespace LeechCraft
{
namespace Monocle
{
	class PageGraphicsItem;
	class IFormField;
	class IFormFieldText;
	class IFormFieldChoice;

	class FormManager : public QObject
	{
		Q_OBJECT

		QGraphicsView * const View_;
		QGraphicsScene * const Scene_;

		QHash<QLineEdit*, std::shared_ptr<IFormFieldText>> Line2Field_;
		QHash<QTextEdit*, std::shared_ptr<IFormFieldText>> Multiline2Field_;
		QHash<QComboBox*, std::shared_ptr<IFormFieldChoice>> Combo2Field_;
	public:
		FormManager (QGraphicsView*, QObject* = 0);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);
	private:
		QGraphicsProxyWidget* AddTextField (std::shared_ptr<IFormField>);
		QGraphicsProxyWidget* AddChoiceField (std::shared_ptr<IFormField>);
	private slots:
		void handleLineEditChanged (const QString&);
		void handleTextEditChanged ();
		void handleComboChanged ();
	};
}
}
