#if defined(LINUX)
#ifndef POWERMATE_H
#define POWERMATE_H

#include <QThread>
#include <QFile>
#include <QObject>
struct input_event;

class PmInput : public QThread {
	Q_OBJECT
	public:
		PmInput(QObject *parent=0, const char *device=0);
		~PmInput();

	protected:
		void run();

	signals:
		void rotated(int n);
		void pressed();
		void released();

	private:
		QString devicename;
		void processEvent(const input_event& event);
		//void setLed(const input_event& event);// set powermate led
};

#endif

#endif
