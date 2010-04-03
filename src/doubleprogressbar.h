/*
 * doubleprogressbar.h
 *
 *  Created on: 2009.05.06.
 *      Author: brazso
 */

#ifndef DOUBLEPROGRESSBAR_H_
#define DOUBLEPROGRESSBAR_H_

#include <QProgressBar>

class DoubleProgressBar: public QProgressBar {
	Q_OBJECT
public:
	DoubleProgressBar(QWidget *parent = 0);
	virtual ~DoubleProgressBar();
	inline double doubleValue() const { return dValue; }
public slots:
	void setDoubleValue(double doubleValue);
	void setDoubleValue(int value);

private:
	double dValue;
};

#endif /* DOUBLEPROGRESSBAR_H_ */
