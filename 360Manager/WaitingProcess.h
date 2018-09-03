#pragma once

#include <string>

#include <QObject>

#include <windows.h>
#include <Xinput.h>

#pragma comment(lib, "Xinput.lib")

using namespace std;

#define XINPUT_GAMEPAD_GUIDE 0x0400

typedef DWORD(WINAPI *XInputGetStateExProc)(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef DWORD(WINAPI *XInputPowerOffControllerProc)(DWORD dwUserIndex);

class WaitingProcess : public QObject
{
	Q_OBJECT
public:
	HMODULE xinputDLL;

	XInputGetStateExProc XInputGetStateEx;
	XInputPowerOffControllerProc XInputPowerOffController;

	XINPUT_STATE state;
	XINPUT_BATTERY_INFORMATION batt;
	XINPUT_KEYSTROKE key;

	DWORD currentState, keyPressed, reserved;

	bool guidePressed, confirmation;

	WaitingProcess(QObject *parent = 0);
	~WaitingProcess();

signals:
	void controller(int index);
	void batteryLevel(int level);
	void showWindow(bool visible);

public slots:
	void waitForGuideButton();

};

