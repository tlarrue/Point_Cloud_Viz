#include "ProgressBar.h"

map<string, pBar> ProgressBar::Bar;

void ProgressBar::Begin(float totalNum, string label) {
	Bar[label].startTime = chrono::steady_clock::now();
	Bar[label].total = totalNum;
	Bar[label].showNextPercent = 10;

	cout << label << ": 0";
}
void ProgressBar::Show(float currentNum, string label) {
	pBar* p = &Bar[label];

	int percent;
	percent = currentNum / p->total;

	if (percent == 100) {
		cout << "100" << endl;
		auto end = chrono::steady_clock::now();
		auto diff = end - p->startTime;
		double totalTime = chrono::duration<double>(diff).count();

		cout << "Total time: " << totalTime << " minutes." << endl;

	} 
	else if (percent >= p->showNextPercent )
	{
		cout << p->showNextPercent;
		p->showNextPercent = p->showNextPercent + 10;
		p->currPercent = percent;
	}
	else {
		int percentDiff = percent - p->currPercent;

		for (int i=0; i < percentDiff; i++) {
			cout << ".";
		}

		p->currPercent = percent;
	}

}