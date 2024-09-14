/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>

class QAction;

namespace LC::Monocle
{
	class IDocument;

	class DocumentActions : public QObject
	{
	public:
		struct Deps
		{
			QWidget& DocTabWidget_;
		};

		struct Separator {};
		using Entry = std::variant<QAction*, QWidget*, Separator>;
	private:
		Deps Deps_;
		IDocument& Doc_;
		QVector<Entry> Entries_;
	public:
		explicit DocumentActions (IDocument& doc, const Deps&);

		const QVector<Entry>& GetEntries () const;
	};
}
