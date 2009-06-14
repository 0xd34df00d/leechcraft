#ifndef PLUGINS_POSHUKU_PROGRESSLINEEDIT_H
#define PLUGINS_POSHUKU_PROGRESSLINEEDIT_H
#include <boost/shared_ptr.hpp>
#include <QLineEdit>

class QModelIndex;
class QToolBar;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class ProgressLineEdit : public QLineEdit
			{
				Q_OBJECT

				bool IsCompleting_;
			public:
				ProgressLineEdit (QWidget* = 0);
				virtual ~ProgressLineEdit ();
				bool IsCompleting () const;
				void AddAction (QAction*);
			protected:
				virtual void focusInEvent (QFocusEvent*);
				virtual void keyPressEvent (QKeyEvent*);
			public slots:
				void handleActivated (const QModelIndex&);
				void handleHighlighted (const QModelIndex&);
			};
		};
	};
};

#endif

