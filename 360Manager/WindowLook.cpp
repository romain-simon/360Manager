#include "WindowLook.h"

#include <QDesktopWidget>
#include <QPainter>
#include <QApplication>
#include <QMainWindow>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QMessageBox>

WindowLook::WindowLook(QWidget *parent) : QWidget(parent)
{
	setupUI();
	setupAnimations();

	wThread = new QThread();
	wProcess = new WaitingProcess();

	wProcess->moveToThread(wThread);
	connect(wThread, SIGNAL(started()), wProcess, SLOT(waitForGuideButton()));
	connect(wProcess, SIGNAL(controller(int)), this, SLOT(changeController(int)));
	connect(wProcess, SIGNAL(batteryLevel(int)), this, SLOT(changeBatteryStatus(int)));
	connect(wProcess, SIGNAL(showWindow(bool)), this, SLOT(setVisible(bool)));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	wThread->start();
}


WindowLook::~WindowLook()
{
	wThread->wait();

	delete wThread;
	delete wProcess;
}

void WindowLook::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);

	groupAni->start();
}

void WindowLook::closeEvent(QCloseEvent * event)
{
	hide();
	event->ignore();
}

void WindowLook::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	QBrush background(QColor(23, 23, 23));

	painter.setBrush(background);
	painter.setPen(Qt::NoPen); // No stroke

	painter.drawRect(0, 0, width(), height());
}

void WindowLook::setupUI()
{
	//
	//	Window management
	//
	resize(512, 170);
	setWindowTitle(tr("360Manager"));
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setWindowIcon(QIcon(":/svg/Resources/icon.svg"));

	//
	//	UI
	//
	QFont font, font1, font2;
	font.setFamily(QStringLiteral("Segoe UI"));
	
	font1.setPointSize(11);
	font2.setPointSize(11);
	font2.setBold(true);
	font2.setWeight(75);
	setFont(font);

	QPalette* palette = new QPalette();
	palette->setColor(QPalette::WindowText, Qt::white);
	setPalette(*palette);

	messageLabel = new QLabel(this);
	messageLabel->setObjectName(QStringLiteral("messageLabel"));
	messageLabel->setGeometry(QRect(170, 30, 341, 31));
	messageLabel->setFont(font1);

	controllerLabel = new QLabel(this);
	controllerLabel->setObjectName(QStringLiteral("controllerLabel"));
	controllerLabel->setGeometry(QRect(20, 20, 131, 131));
	controllerLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/controller.svg")));
	controllerLabel->setScaledContents(true);

	batteryLabel = new QLabel(this);
	batteryLabel->setObjectName(QStringLiteral("batteryLabel"));
	batteryLabel->setGeometry(QRect(65, 100, 41, 41));
	batteryLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/empty_battery.svg")));
	batteryLabel->setScaledContents(true);

	controllerIndexLabel = new QLabel(this);
	controllerIndexLabel->setObjectName(QStringLiteral("controllerIndexLabel"));
	controllerIndexLabel->setGeometry(QRect(460, 120, 40, 40));
	controllerIndexLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/xbox_0.svg")));
	controllerIndexLabel->setScaledContents(true);

	gridLayoutWidget = new QWidget(this);
	gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
	gridLayoutWidget->setGeometry(QRect(190, 80, 271, 33));

	gridLayout_2 = new QGridLayout(gridLayoutWidget);
	gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
	gridLayout_2->setContentsMargins(0, 0, 0, 0);

	aButtonLabel = new QLabel(gridLayoutWidget);
	aButtonLabel->setObjectName(QStringLiteral("aButtonLabel"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(aButtonLabel->sizePolicy().hasHeightForWidth());
	aButtonLabel->setSizePolicy(sizePolicy);
	aButtonLabel->setMinimumSize(QSize(31, 31));
	aButtonLabel->setMaximumSize(QSize(31, 31));
	aButtonLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/A_button.svg")));
	aButtonLabel->setScaledContents(true);

	bButtonLabel = new QLabel(gridLayoutWidget);
	bButtonLabel->setObjectName(QStringLiteral("bButtonLabel"));
	bButtonLabel->setMinimumSize(QSize(31, 31));
	bButtonLabel->setMaximumSize(QSize(31, 31));
	bButtonLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/B_button.svg")));
	bButtonLabel->setScaledContents(true);

	yesLabel = new QLabel(gridLayoutWidget);
	yesLabel->setObjectName(QStringLiteral("yesLabel"));
	yesLabel->setFont(font2);

	cancelLabel = new QLabel(gridLayoutWidget);
	cancelLabel->setObjectName(QStringLiteral("cancelLabel"));
	cancelLabel->setFont(font2);

	horizontalSpacer = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	gridLayout_2->addWidget(aButtonLabel, 0, 0, 1, 1);
	gridLayout_2->addWidget(yesLabel, 0, 1, 1, 1);
	gridLayout_2->addItem(horizontalSpacer, 0, 3, 1, 1);
	gridLayout_2->addWidget(bButtonLabel, 0, 4, 1, 1);
	gridLayout_2->addWidget(cancelLabel, 0, 5, 1, 1);

	retranslateUi();

	//
	//	System Tray
	//
	QMenu *trayIconMenu = new QMenu(this);

	QAction *aboutAct = new QAction(tr("&About"), this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(showAbout()));

	QAction *quitAct = new QAction(tr("&Quit"), this);
	connect(quitAct, SIGNAL(triggered()), this, SLOT(exitApp()));

	trayIconMenu->addAction(aboutAct);
	trayIconMenu->addAction(quitAct);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(QIcon(":/svg/Resources/icon.svg"));
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->show();
}

void WindowLook::setupAnimations()
{
	QDesktopWidget screen;

	QRect screenGeom = screen.screenGeometry(this);

	int screenCenterX = screenGeom.center().x();
	int bottom = screenGeom.bottom();

	QPropertyAnimation *animOpa = new QPropertyAnimation(this, "windowOpacity");
	animOpa->setStartValue(0.0);
	animOpa->setEndValue(1.0);
	animOpa->setEasingCurve(QEasingCurve::InBack);
	animOpa->setDuration(250);

	QPropertyAnimation *animMov = new QPropertyAnimation(this, "geometry");
	animMov->setStartValue(QRect((screenCenterX - width() / 2), bottom - height(), width(), height()));
	animMov->setEndValue(QRect((screenCenterX - width() / 2), (bottom - height() * 2), width(), height()));
	animMov->setDuration(250);

	groupAni = new QParallelAnimationGroup();
	groupAni->addAnimation(animOpa);
	groupAni->addAnimation(animMov);
}

void WindowLook::retranslateUi()
{
	messageLabel->setText(tr("Do you want to power off the controller?"));
	cancelLabel->setText(tr("Cancel"));
	yesLabel->setText(tr("Yes"));
}

void WindowLook::iconActivated(QSystemTrayIcon::ActivationReason reason) 
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger: show();
	default:
		break;
	}
}

void WindowLook::changeController(int index)
{
	switch (index) 
	{
	case 0: controllerIndexLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/xbox_1.svg")));
		break;
	case 1: controllerIndexLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/xbox_2.svg")));
		break;
	case 2: controllerIndexLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/xbox_3.svg")));
		break;
	case 3: controllerIndexLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/xbox_4.svg")));
		break;
	default: controllerIndexLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/xbox_0.svg")));
	}
}

void WindowLook::changeBatteryStatus(int level)
{
	switch (level)
	{
	case 0: batteryLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/empty_battery.svg")));
		break;
	case 1: batteryLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/low_battery.svg")));
		break;
	case 2: batteryLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/half_full_battery.svg")));
		break;
	case 3: batteryLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/full_battery.svg")));
		break;
	default: batteryLabel->setPixmap(QPixmap(QString::fromUtf8(":/svg/Resources/empty_battery.svg")));
	}
}

void WindowLook::showAbout()
{
	QMessageBox aboutWindow;

	QString aboutText = "<h3>360Manager</h3>" 
		+ QString("<p>Version 0.1.0<br/>")
		+ QString("Copyright ") + QChar(0x00A9) + QString(" 2017 CosmosDev. All rights reserved.</p>")
		+ QString("<h4>Credits</h4>") 
		+ QString("<p>Controller illustration made by Freepik from www.flaticon.com<br/>")
		+ QString("Battery illustrations made by Icon Works from www.flaticon.com<p>")
		+ QString("<p>Xbox") + QChar(0x00AE) + QString(" and its logo are registered trademarks of Microsoft Corporation.<br/>")
		+ QString("I am not affiliated with Microsoft Corporation.<p>");

	aboutWindow.about(this, tr("About 360Manager"), aboutText);
	aboutWindow.setVisible(true);
}

void WindowLook::exitApp()
{
	exit(EXIT_SUCCESS);
}

