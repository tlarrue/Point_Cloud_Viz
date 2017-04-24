#pragma once
#include <map>
#include <string>
#include <chrono>
#include <vector>
#include <iostream>
using namespace std;

struct pBar {
	float total;
	float current;
	int currPercent;
	int showNextPercent;
	chrono::steady_clock::time_point startTime;
	double totalTime;
};

class ProgressBar {
private:
	static map<string, pBar> Bar;
public:
	static void Begin(float totalNum, string label = "Progress Bar");
	static void Show(float currentNum, string label = "Progress Bar");
};