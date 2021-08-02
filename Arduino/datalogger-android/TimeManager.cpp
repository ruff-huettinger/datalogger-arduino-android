#include "TimeManager.h"

void TimeManager::updateTimes()
{
	updateCurrentTime();

	nextStateChangeTime = getNextStateChangeTime(currentTime.tm_min, activeTable[currentTime.tm_hour].numOfMeasures);
	//nextStateChangeTime = getNextStateChangeTimeDebug(currentTime.tm_sec, activeTable[currentTime.tm_min].numOfMeasures);
}

void TimeManager::updateCurrentTime()
{
	unixSecs = time(NULL);
	currentTime = *localtime(&unixSecs);
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

tm TimeManager::getNextStateChangeTimeDebug(uint8_t currentSecond, uint8_t measuresPerMinute) {
	uint8_t* times = new uint8_t[measuresPerMinute];
	uint8_t val = 0;
	for (int i = 0; i <= measuresPerMinute; i++) {
		times[i] = i * (60 / measuresPerMinute);
		if (times[i] > currentSecond) {
			val = times[i];
			break;
		}
	}
	delete[] times;
	time_t seconds = time(NULL);
	tm t = *localtime(&seconds);
	t.tm_sec = val;
	return t;
}



double TimeManager::getDiffTime()
{
	updateTimes();
	time_t now = mktime(&currentTime);
	time_t then = mktime(&nextStateChangeTime);
	double timeDif = difftime(then, now);
	return timeDif;
}

void TimeManager::getTimeStampSecs(char* secs)
{
	updateCurrentTime();
	secs = ctime(&unixSecs);
}

void TimeManager::getTimeStampDate(char* date)
{
	updateCurrentTime();
	strftime(date, 32, "%Y-%m-%d_%H-%M-%S", &currentTime);
}

void TimeManager::getTimeStampTime(char* time)
{
	updateCurrentTime();
	strftime(time, 32, "%I_%M_%S", &currentTime);
}

void TimeManager::getDateTimeFormatted(char* time)
{
	updateCurrentTime();
	strftime(time, 32, "%x %X", &currentTime);
}

tm* TimeManager::getCurrentTime()
{
	updateCurrentTime();
	return &currentTime;
}




