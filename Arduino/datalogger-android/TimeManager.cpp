#include "TimeManager.h"

void TimeManager::updateTimes()
{
	updateCurrentTime();
	nextStateChangeTime_ = getNextStateChangeTime(currentTime_.tm_min, activeTable[currentTime_.tm_hour].numOfMeasures);
}

void TimeManager::updateCurrentTime()
{
	unixSecs_ = time(NULL);
	currentTime_ = *localtime(&unixSecs_);
}

tm TimeManager::getNextStateChangeTime(uint8_t currentMinute, uint8_t measuresPerHour) {
	uint8_t* times = new uint8_t[measuresPerHour];
	uint8_t val = 0;
	for (int i = 0; i < measuresPerHour; i++) {
		times[i] = i * (60 / measuresPerHour);
		if (times[i] > currentMinute) {
			val = times[i];
			break;
		}
	}
	delete[] times;
	time_t seconds = time(NULL);
	tm t = *localtime(&seconds);
	t.tm_min = val;
	if (val == 0) {
		t.tm_hour = (t.tm_hour + 1);
	}
	t.tm_sec = 0;
	return t;
}

double TimeManager::getDiffTime()
{
	updateTimes();
	time_t now = mktime(&currentTime_);
	time_t then = mktime(&nextStateChangeTime_);
	double timeDif = difftime(then, now);
	return timeDif;
}

void TimeManager::getTimeStampSecs(char* secs)
{
	updateCurrentTime();
	secs = ctime(&unixSecs_);
}

void TimeManager::getTimeStampDate(char* date)
{
	updateCurrentTime();
	strftime(date, 32, "%Y-%m-%d_%H-%M-%S", &currentTime_);
}

void TimeManager::getTimeStampTime(char* time)
{
	updateCurrentTime();
	strftime(time, 32, "%I_%M_%S", &currentTime_);
}

void TimeManager::getDateTimeFormatted(char* time)
{
	updateCurrentTime();
	strftime(time, 32, "%x %X", &currentTime_);
}

tm* TimeManager::getCurrentTime()
{
	updateCurrentTime();
	return &currentTime_;
}




