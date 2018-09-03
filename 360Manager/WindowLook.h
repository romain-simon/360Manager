#pragma once

#include <QtWidgets\QWidget>
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>
#include <QParallelAnimationGroup>
#include <QCloseEvent>
#include <QTranslator>

#include "WaitingProcess.h"

class WindowLook : public QWidget
{
	Q_OBJECT

public:
	WindowLook(QWidget *parent = 0);
	~WindowLook();

protected:
	void showEvent(QShowEvent *event);
	void closeEvent(QCloseEvent *event);
	void paintEvent(QPaintEvent *event);

private:
	QWidget *centralwidget;
	QWidget *gridLayoutWidget;

	QLabel *messageLabel;
	QLabel *controllerLabel;
	QLabel *batteryLabel;
	QLabel *bButtonLabel;
	QLabel *cancelLabel;
	QLabel *yesLabel;
	QLabel *aButtonLabel;
	QLabel *controllerIndexLabel;

	QGridLayout *gridLayout_2;

	QSpacerItem *horizontalSpacer;

	QSystemTrayIcon *trayIcon;

	QParallelAnimationGroup *groupAni;

	QThread *wThread;
	WaitingProcess *wProcess;

	void setupUI();
	void setupAnimations();
	void retranslateUi();

private slots:
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	void changeController(int index);
	void changeBatteryStatus(int level);
	void showAbout();
	void exitApp();
};

