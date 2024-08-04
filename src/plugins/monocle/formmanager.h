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
class QButtonGroup;

namespace LC::Monocle
{
	struct LinkExecutionContext;
	class PageGraphicsItem;

	class IDocument;
	class IFormField;
	class IFormFieldButton;

	class FormManager : public QObject
	{
		LinkExecutionContext& ExecutionContext_;
		QGraphicsScene * const Scene_;

		QHash<QList<int>, std::shared_ptr<QButtonGroup>> RadioGroups_;
	public:
		explicit FormManager (QGraphicsView*, LinkExecutionContext&);

		void HandleDoc (IDocument&, const QVector<PageGraphicsItem*>&);
	private:
		QGraphicsProxyWidget* AddTextField (const std::shared_ptr<IFormField>&);
		QGraphicsProxyWidget* AddChoiceField (const std::shared_ptr<IFormField>&);
		QGraphicsProxyWidget* AddButtonField (const std::shared_ptr<IFormField>&);
	};
}
