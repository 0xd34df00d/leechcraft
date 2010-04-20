#ifndef PLUGINS_POC_POC_H
#define PLUGINS_POC_POC_H
#include <interfaces/iinfo.h>
#include <interfaces/itoolbarembedder.h>
#include <interfaces/iembedtab.h>

class Editor;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PoC
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IToolBarEmbedder
						 , public IEmbedTab
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IToolBarEmbedder IEmbedTab)

			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);
				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;

				QList<QAction*> GetActions () const;
			private:
				QList<QAction*> Actions_;
				Editor * poc;
			signals:
				void bringToFront ();
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
			};
		};
	};
};

#endif
