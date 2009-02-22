#include "sourceviewer.h"
#include "htmlhighlighter.h"

SourceViewer::SourceViewer (QWidget *parent)
: QMainWindow (parent)
{
	Ui_.setupUi (this);
	new HtmlHighlighter (Ui_.HtmlEdit_);
}

void SourceViewer::SetHtml (const QString& html)
{
	Ui_.HtmlEdit_->setPlainText (html);
}

