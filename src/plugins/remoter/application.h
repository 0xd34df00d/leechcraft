#ifndef APPLICATION_H
#define APPLICATION_H
#include <WApplication>

class Application : public Wt::WApplication
{
public:
	Application (const Wt::WEnvironment&);
private:
	bool Ajax_;
};

Wt::WApplication* ApplicationCreator (const Wt::WEnvironment&);

#endif

