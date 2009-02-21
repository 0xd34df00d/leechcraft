#ifndef APPLICATION_H
#define APPLICATION_H
#include <memory>
#include <QApplication>
#include <QStringList>
#include <QTranslator>

namespace LeechCraft
{
	class Application : public QApplication
	{
		Q_OBJECT

		QStringList Arguments_;

		enum Errors
		{
			EAlreadyRunning = 1,
			EPaths = 2,
			EHelpRequested = 3
		};

		std::auto_ptr<QTranslator> Translator_;
	public:
		Application (int&, char**);
		const QStringList& Arguments () const;

		virtual bool notify (QObject*, QEvent*);
	private:
		bool IsAlreadyRunning () const;
		void ParseCommandLine ();
	};
};

#endif

