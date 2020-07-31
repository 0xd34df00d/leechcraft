/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_IMPORTBINARY_H
#define PLUGINS_AGGREGATOR_IMPORTBINARY_H
#include <QDialog>
#include "ui_importbinary.h"
#include "feed.h"
#include "channel.h"

namespace LC
{
namespace Aggregator
{
	class ImportBinary : public QDialog
	{
		Q_OBJECT

		Ui::ImportBinary Ui_;
		channels_container_t Channels_;
	public:
		ImportBinary (QWidget* = 0);
		virtual ~ImportBinary ();
		QString GetFilename () const;
		QString GetTags () const;
		feeds_container_t GetSelectedFeeds () const;
	private slots:
		void on_File__textEdited (const QString&);
		void on_Browse__released ();
	private:
		bool HandleFile (const QString&);
		void Reset ();
	};
}
}

#endif
