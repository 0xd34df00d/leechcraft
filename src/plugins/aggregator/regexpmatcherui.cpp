#include "regexpmatcherui.h"

RegexpMatcherUi::RegexpMatcherUi ()
{
	Ui_.setupUi (this);
}

RegexpMatcherUi::~RegexpMatcherUi ()
{
}

RegexpMatcherUi& RegexpMatcherUi::Instance ()
{
	static RegexpMatcherUi rmu;
	return rmu;
}

void RegexpMatcherUi::Release ()
{
}

