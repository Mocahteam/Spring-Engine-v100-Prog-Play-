#include "Event.h"
#include "TraceConstantList.h"

const char* Event::concatEventsArr[] = {GAME_PAUSED, GAME_UNPAUSED, NULL};
const char* Event::noConcatEventsArr[] = {START_MISSION, END_MISSION, NEW_EXECUTION, END_EXECUTION, "eof", NULL};

Event::Event(std::string label, std::string info) : Trace(EVENT,info), label(label) {}

Event::Event(const Event *e) : Trace(e) {
	label = e->getLabel();
}

bool Event::operator==(Trace *t) const {
	bool res = false;
	if (t->isEvent()) {
		Event *e = dynamic_cast<Event*>(t);
		if (label.compare(e->label) == 0)
			res = true;
	}
	return res;
}

Trace::sp_trace Event::clone() const {
	return boost::make_shared<Event>(this);
}

void Event::display(std::ostream &os) const {
	for (int i = 0; i <= numTab; i++)
		os << "\t";
	os << label;
	os << " " << getParams() << std::endl;
}

unsigned int Event::length() const {
	return 0;
}

std::string Event::getLabel() const {
	return label;
}

std::string Event::getParams() const {
	return "";
}
