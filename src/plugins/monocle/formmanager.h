/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
class QTreeWidget;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QButtonGroup;

namespace LC
{
namespace Monocle
{
	class PageGraphicsItem;
	class IFormField;
	class IFormFieldText;
	class IFormFieldChoice;
	class IFormFieldButton;

	class FormManager : public QObject
	{
		Q_OBJECT

		QGraphicsView * const View_;
		QGraphicsScene * const Scene_;

		QHash<QLineEdit*, std::shared_ptr<IFormFieldText>> Line2Field_;
		QHash<QTextEdit*, std::shared_ptr<IFormFieldText>> Multiline2Field_;
		QHash<QComboBox*, std::shared_ptr<IFormFieldChoice>> Combo2Field_;
		QHash<QTreeWidget*, std::shared_ptr<IFormFieldChoice>> List2Field_;
		QHash<QCheckBox*, std::shared_ptr<IFormFieldButton>> Check2Field_;
		QHash<QRadioButton*, std::shared_ptr<IFormFieldButton>> Radio2Field_;
		QHash<QPushButton*, std::shared_ptr<IFormFieldButton>> Button2Field_;

		QHash<QList<int>, QButtonGroup*> RadioGroups_;
	public:
		FormManager (QGraphicsView*, QObject* = 0);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);
	private:
		QGraphicsProxyWidget* AddTextField (std::shared_ptr<IFormField>);
		QGraphicsProxyWidget* AddChoiceField (std::shared_ptr<IFormField>);
		QGraphicsProxyWidget* AddButtonField (std::shared_ptr<IFormField>);
	private slots:
		void handleLineEditChanged (const QString&);
		void handleTextEditChanged ();

		void handleComboChanged ();
		void handleListChanged ();

		void handleCheckboxChanged ();
		void handleRadioChanged ();
		void handleButtonReleased ();
	};
}
}
