#include "pane.h"
#include <QDirModel>
#include <QCompleter>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Pane::Pane (QWidget *parent)
			: QWidget (parent)
			, DirModel_ (new QDirModel (this))
			{
				Ui_.setupUi (this);
				Ui_.Address_->setCompleter (new QCompleter ());

				DirModel_->setSorting (QDir::DirsFirst | QDir::IgnoreCase);
			}

			Pane::~Pane ()
			{
			}

			void Pane::SetURL (const QUrl& url)
			{
				if (Ui_.Address_->completer ()->model ())
					Ui_.Address_->completer ()->setModel (0);
			}

			void Pane::Navigate (const QString& string)
			{
				if (!Ui_.Address_->completer ()->model ())
					Ui_.Address_->completer ()->setModel (DirModel_);

				Ui_.Tree_->setModel (DirModel_);
				Ui_.Tree_->setRootIndex (DirModel_->index (string));
			}
		};
	};
};

