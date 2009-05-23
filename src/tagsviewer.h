#ifndef TAGSVIEWER_H
#define TAGSVIEWER_H
#include <QWidget>
#include "ui_tagsviewer.h"

namespace LeechCraft
{
	class TagsViewer : public QWidget
	{
		Q_OBJECT

		Ui::TagsViewer Ui_;
	public:
		TagsViewer (QWidget* = 0);
	private slots:
		void on_Rename__released ();
		void on_Remove__released ();
	};
};

#endif

