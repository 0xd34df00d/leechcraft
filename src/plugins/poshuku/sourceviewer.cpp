#include "sourceviewer.h"
#include "htmlhighlighter.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
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
		};
	};
};

