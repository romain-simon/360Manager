#include "WaitingProcess.h"

using namespace std;

WaitingProcess::WaitingProcess(QObject *parent) : QObject(parent)
{
	xinputDLL = LoadLibrary(L"xinput1_3.dll");

	if (xinputDLL == NULL) {
		xinputDLL = LoadLibrary(L"xinput1_4.dll");
	}

	XInputGetStateEx = (XInputGetStateExProc)GetProcAddress(xinputDLL, (LPCSTR)100);
	XInputPowerOffController = (XInputPowerOffControllerProc)GetProcAddress(xinputDLL, (LPCSTR)103);

	ZeroMemory(&state, sizeof(XINPUT_STATE));
	ZeroMemory(&batt, sizeof(XINPUT_BATTERY_INFORMATION));
	ZeroMemory(&key, sizeof(XINPUT_KEYSTROKE));

	reserved = NULL;
}


WaitingProcess::~WaitingProcess()
{
}

void WaitingProcess::waitForGuideButton()
{
	while (1) {
		for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
			currentState = XInputGetStateEx(i, &state);

			guidePressed = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE) != 0);

			if (guidePressed && currentState == ERROR_SUCCESS) {
				XInputGetBatteryInformation(i, BATTERY_DEVTYPE_GAMEPAD, &batt);
				emit controller(i);
				emit batteryLevel((int)batt.BatteryLevel);
				emit showWindow(true);
				confirmation = false;
				while (!confirmation) {
					keyPressed = XInputGetKeystroke(i, reserved, &key);
					if (keyPressed == ERROR_SUCCESS && key.VirtualKey == VK_PAD_A && key.Flags == XINPUT_KEYSTROKE_KEYDOWN) {
						XInputPowerOffController(i);
						emit showWindow(false);
						confirmation = true;
					}
					else if (keyPressed == ERROR_SUCCESS && key.VirtualKey == VK_PAD_B && key.Flags == XINPUT_KEYSTROKE_KEYDOWN) {
						emit showWindow(false);
						confirmation = true;
					}
				}
			}
			Sleep(10);
		}
	}
}
