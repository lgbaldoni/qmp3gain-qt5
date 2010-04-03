/*
 * doubleprogressbar.cpp
 *
 *  Created on: 2009.05.06.
 *      Author: brazso
 */

#include "doubleprogressbar.h"

DoubleProgressBar::DoubleProgressBar(QWidget *parent): QProgressBar(parent) {
	// TODO Auto-generated constructor stub
	this->dValue = (double)this->value();
	// setValue(int) must update dValue
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(setDoubleValue(int)));
}

DoubleProgressBar::~DoubleProgressBar() {
	// TODO Auto-generated destructor stub
}

void DoubleProgressBar::setDoubleValue(double doubleValue) {
	this->dValue = doubleValue;
	this->setValue((int)doubleValue);
}

void DoubleProgressBar::setDoubleValue(int value) {
	setDoubleValue((double)value);
}
