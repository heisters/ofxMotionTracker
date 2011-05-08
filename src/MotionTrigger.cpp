/*
 *  MotionTrigger.cpp
 *  openFrameworks
 *
 *  Created by Pat Long on 06/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "MotionTrigger.h"

MotionTrigger::MotionTrigger(){
}

MotionTrigger::~MotionTrigger(){
	for(vector<AngleTrigger*>::iterator it = this->angleTriggers.begin(); it != this->angleTriggers.end(); it++)
		delete (*it);
}

void MotionTrigger::init(int cameraWidth, int cameraHeight, int cameraID){
	MotionTracker::init(cameraWidth, cameraHeight, cameraID);
	this->setMirrorVideo(true);
	this->triggerDelay = 0;
	this->fireTime = -1;
	this->triggerArea = NULL;
}

void MotionTrigger::fire(){
	this->firing = true;
	this->fireTime = -1;
}

void MotionTrigger::ceaseFire(){
	this->firing = false;
}

bool MotionTrigger::isReadyToFire(){
	if(this->triggerDelay == 0 && this->shouldTrigger())
		return true;
	
	int currentTime = ofGetElapsedTimeMillis();
	if(this->fireTime <= 0 && this->shouldTrigger())
		this->fireTime = currentTime + this->triggerDelay;
	else if(this->fireTime > 0 && (currentTime - this->fireTime) >= this->triggerDelay)
		return true;
	
	return false;
}

bool MotionTrigger::checkAngleTriggers(float angle, float magnitude){
	for(vector<AngleTrigger*>::iterator it = this->angleTriggers.begin(); it != this->angleTriggers.end(); it++){
		if((*it)->shouldTrigger(angle, magnitude))
			return true;
	}
	return false;
}

bool MotionTrigger::shouldTrigger(){
	float angle, magnitude;
	this->getVelAverageAngleMag(&angle, &magnitude, this->triggerArea);
	return this->checkAngleTriggers(angle, magnitude);
}

bool MotionTrigger::isFiring(){
	return this->firing;
}

int MotionTrigger::addAngleTrigger(float triggerAngle, float triggerAngleSpread, float triggerMagnitudeMin, float triggerMagnitudeMax){
	AngleTrigger* newTrigger = new AngleTrigger();
	this->angleTriggers.push_back(newTrigger);
	newTrigger->setTriggerParams(triggerAngle, triggerAngleSpread, triggerMagnitudeMin, triggerMagnitudeMax);
	return (this->angleTriggers.size()-1);
}

void MotionTrigger::incrementAngleTriggerRanges(float increment){
	for(vector<AngleTrigger*>::iterator it = this->angleTriggers.begin(); it != this->angleTriggers.end(); it++)
		(*it)->resizeAngleRange(increment);
}


int MotionTrigger::getTriggerDelay(){
	return this->triggerDelay;
}

MotionTrigger::AngleTrigger* MotionTrigger::getAngleTrigger(int triggerID){
	if(triggerID < 0 || triggerID >= this->angleTriggers.size())
		return NULL;
	else
		return this->angleTriggers[triggerID];
}

float MotionTrigger::getAngleTriggerAngle(int triggerID){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		return trigger->getTriggerAngle();
	return 0;
}

float MotionTrigger::getAngleTriggerAngleSpread(int triggerID){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		return trigger->getTriggerAngleSpread();
	return 0;
}

float MotionTrigger::getAngleTriggerMagnitudeMin(int triggerID){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		return trigger->getTriggerMagnitudeMin();
	return 0;
}

float MotionTrigger::getAngleTriggerMagnitudeMax(int triggerID){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		return trigger->getTriggerMagnitudeMax();
	return 0;
}

void MotionTrigger::setTriggerAngle(int triggerID, float triggerAngle){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		trigger->setTriggerAngle(triggerAngle);
}

void MotionTrigger::setTriggerAngleSpread(int triggerID, float triggerAngleSpread){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		trigger->setTriggerAngleSpread(triggerAngleSpread);
}

void MotionTrigger::setTriggerMagnitudeRange(int triggerID, float triggerMagnitudeMin, float triggerMagnitudeMax){
	AngleTrigger* trigger = this->getAngleTrigger(triggerID);
	if(trigger != NULL)
		trigger->setTriggerMagnitudeRange(triggerMagnitudeMin, triggerMagnitudeMax);
}

void MotionTrigger::setTriggerArea(ofRectangle* triggerArea){
	this->triggerArea = triggerArea;
}

void MotionTrigger::setTriggerDelay(int triggerDelay){
	this->triggerDelay = triggerDelay;
}

void MotionTrigger::setAngleTriggerMagnitudeRanges(float magnitudeMin, float magnitudeMax){
	for(vector<AngleTrigger*>::iterator it = this->angleTriggers.begin(); it != this->angleTriggers.end(); it++)
		(*it)->setTriggerMagnitudeRange(magnitudeMin, magnitudeMax);
}

void MotionTrigger::draw(){
	this->drawDebugVideo(0, 480, 320, 240);
	this->drawColor(0, 0, 640, 480);
	this->drawOpticalFlowTriggers(640, 0, 640, 480);
}

void MotionTrigger::drawAngleTriggers(int x, int y, int w, int h, float ratioX, float ratioY){
	for(vector<AngleTrigger*>::iterator it = this->angleTriggers.begin(); it != this->angleTriggers.end(); it++)
		(*it)->drawTriggerAngleRange(x, y, w, h, ratioX, ratioY);
}

void MotionTrigger::drawOpticalFlowTriggers(int x, int y, int w, int h){
	float ratioX = ((float)w / (float)this->camWidth) / 2.0;
	float ratioY = ((float)h / (float)this->camHeight) / 2.0;
	float centerX = x + w/2.0;
	float centerY = y + h/2.0;
	
	ofSetColor(255, 255, 255);
	this->drawOpticalFlowAverage(x, y, w, h, this->triggerArea);
	this->drawAngleTriggers(x, y, w, h, ratioX, ratioY);
	
	if(this->isFiring())
		ofCircle(centerX, centerY, 10);
	ofSetColor(255, 255, 255);
}

void MotionTrigger::update(){
	MotionTracker::update();
	if(this->isReadyToFire())
		this->fire(); // FIRE!!!
	else
		this->ceaseFire();
}


MotionTrigger::AngleTrigger::AngleTrigger(){
	this->setTriggerParams();
}

MotionTrigger::AngleTrigger::AngleTrigger(float triggerAngle, float triggerAngleSpread, float triggerMagnitudeMin, float triggerMagnitudeMax){
	this->setTriggerParams(triggerAngle, triggerAngleSpread, triggerMagnitudeMin, triggerMagnitudeMax);
}

MotionTrigger::AngleTrigger::~AngleTrigger(){
}

bool MotionTrigger::AngleTrigger::angleInRange(float angle){
	bool check = false;
	float angleLow = tiNormalizeAngle(this->triggerAngle - this->triggerAngleSpread);
	float angleHigh = tiNormalizeAngle(this->triggerAngle + this->triggerAngleSpread);
	
	if(angleLow <= angleHigh){
		if(angle >= angleLow && angle <= angleHigh)
			check = true;
	}
	else{
		// an angle bounds case (ie. handle the 0/360 loop around)
		if((angle >= angleLow && angle <= 360.0) || (angle >= 0.0 && angle <= angleHigh))
			check = true;
	}	
	return check;
}

bool MotionTrigger::AngleTrigger::magnitudeInRange(float magnitude){
	return (magnitude >= this->triggerMagnitudeMin && magnitude <= this->triggerMagnitudeMax);
}

bool MotionTrigger::AngleTrigger::shouldTrigger(float angle, float magnitude){
	if(this->magnitudeInRange(magnitude))
		return this->angleInRange(angle);
	return false;
}

float MotionTrigger::AngleTrigger::getTriggerAngle(){
	return this->triggerAngle;
}

float MotionTrigger::AngleTrigger::getTriggerAngleSpread(){
	return this->triggerAngleSpread;
}

float MotionTrigger::AngleTrigger::getTriggerMagnitudeMin(){
	return this->triggerMagnitudeMin;
}

float MotionTrigger::AngleTrigger::getTriggerMagnitudeMax(){
	return this->triggerMagnitudeMax;
}

void MotionTrigger::AngleTrigger::resizeAngleRange(float increment){
	this->setTriggerAngleSpread(this->triggerAngleSpread + increment);
}

void MotionTrigger::AngleTrigger::setTriggerAngle(float triggerAngle){
	this->triggerAngle = triggerAngle;
}

void MotionTrigger::AngleTrigger::setTriggerAngleSpread(float triggerAngleSpread){
	this->triggerAngleSpread = triggerAngleSpread;
}

void MotionTrigger::AngleTrigger::setTriggerMagnitudeRange(float triggerMagnitudeMin, float triggerMagnitudeMax){
	this->triggerMagnitudeMin = triggerMagnitudeMin;
	this->triggerMagnitudeMax = triggerMagnitudeMax;
}

void MotionTrigger::AngleTrigger::setTriggerParams(float triggerAngle, float triggerAngleSpread, float triggerMagnitudeMin, float triggerMagnitudeMax){
	this->setTriggerAngle(triggerAngle);
	this->setTriggerAngleSpread(triggerAngleSpread);
	this->setTriggerMagnitudeRange(triggerMagnitudeMin, triggerMagnitudeMax);
}

void MotionTrigger::AngleTrigger::drawTriggerAngleRange(int x, int y, int w, int h, float ratioX, float ratioY){
	float centerX = x + w/2.0;
	float centerY = y + h/2.0;
	float x2, y2;
	float lowAngle = tiNormalizeAngle(this->triggerAngle - this->triggerAngleSpread);
	x2 = centerX + tiAngleXComponent(lowAngle, this->triggerMagnitudeMax * ratioX);
	y2 = centerY + tiAngleYComponent(lowAngle, this->triggerMagnitudeMax * ratioY);
	ofLine(centerX, centerY, x2, y2);

	float highAngle = tiNormalizeAngle(this->triggerAngle + this->triggerAngleSpread);
	x2 = centerX + tiAngleXComponent(highAngle, this->triggerMagnitudeMax * ratioX);
	y2 = centerY + tiAngleYComponent(highAngle, this->triggerMagnitudeMax * ratioY);
	ofLine(centerX, centerY, x2, y2);

#ifdef DRAW_TRIGGER_ARC
	ofNoFill();
	ofSetCircleResolution(360);
	ofCircleSlice(centerX, centerY, this->triggerMagnitudeMin * ratioX, lowAngle, highAngle);
	ofCircleSlice(centerX, centerY, this->triggerMagnitudeMax * ratioX, lowAngle, highAngle);
	ofSetCircleResolution(CIRC_RESOLUTION);
	ofFill();
#endif
}
