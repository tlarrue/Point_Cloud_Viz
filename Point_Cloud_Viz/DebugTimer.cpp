// #include "header.h"
#include "DebugTimer.h"

map<string, SingleTimePoint> DebugTimer::Map;

void DebugTimer::Begin(string label) {
#ifdef DEBUG_TIMER_ON
	Map[label].startTime = chrono::steady_clock::now();
#endif // DEBUG_TIMER_ON
}
void DebugTimer::End(string label) {
#ifdef DEBUG_TIMER_ON
	SingleTimePoint* t = &Map[label];
	auto end = chrono::steady_clock::now();
	auto diff = end - t->startTime;
	t->totalTime += chrono::duration <double, milli>(diff).count();

	if (t->counter == TOTAL_TIMERS - 1)
	{
		cout << "[" << label << "]\t" << t->totalTime / TOTAL_TIMERS << " ms" << endl;
		t->counter = 0;
		t->totalTime = 0;
	}
	else
		t->counter++;

	//cout << "[" << label << "]\t" << chrono::duration <double, milli>(diff).count() << " ms" << endl;
	//cout << counter << endl;
#endif // DEBUG_TIMER_ON
}