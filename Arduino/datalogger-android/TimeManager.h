#pragma once

#include "config.h"
#include "StateTable.h"
#include <mbed.h>

/**
 * @file TimeManager.h
 *
 * - uses mbed-time for accurate real-world timing
 * - calculates the time until the next measuring
 * - gives out differently formatted timestamps
 *
 * @author Johannes Brunner
 *
 */


class TimeManager
{
private:
	tm currentTime_;
	tm nextStateChangeTime_;
	time_t unixSecs_;

	void updateTimes();
	void updateCurrentTime();
	tm getNextStateChangeTime(uint8_t currentMinute, uint8_t measuresPerHour);

public:
	double getDiffTime();

	void getTimeStampSecs(char* secs);
	void getTimeStampDate(char* date);
	void getTimeStampTime(char* time);
	tm* getCurrentTime();
	void getDateTimeFormatted(char* time);

	TimeManager() {};
	~TimeManager() {};
};

