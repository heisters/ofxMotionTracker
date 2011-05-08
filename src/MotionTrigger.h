/*
 *  MotionTrigger.h
 *  openFrameworks
 *
 *  Created by Pat Long on 06/04/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _MOTION_TRIGGER
#define _MOTION_TRIGGER

#include "ofxMotionTracker.h"
#include "ofCircleSlice.h"

#define DEFAULT_TRIGGER_ANGLE 90
#define DEFAULT_TRIGGER_ANGLE_SPREAD 90
#define DEFAULT_TRIGGER_MAGNITUDE_MIN 5.0
#define DEFAULT_TRIGGER_MAGNITUDE_MAX 500.0

#define DRAW_TRIGGER_ARC // requires the ofGraphics::ofCircleSlice() mod from http://www.openframeworks.cc/forum/viewtopic.php?t=1811

class MotionTrigger : public MotionTracker{
public:
	class AngleTrigger{
		private:
			float triggerAngle, triggerAngleSpread, triggerMagnitudeMin, triggerMagnitudeMax;
		
			bool angleInRange(float angle);
			bool magnitudeInRange(float magnitude);
		
		public:
			AngleTrigger();
			AngleTrigger(float triggerAngle, float triggerAngleSpread, float triggerMagnitudeMin, float triggerMagnitudeMax);
			~AngleTrigger();
		
			bool shouldTrigger(float angle, float magnitude);

			float getTriggerAngle();
			float getTriggerAngleSpread();
			float getTriggerMagnitudeMin();
			float getTriggerMagnitudeMax();
		
			void resizeAngleRange(float increment);
			void setTriggerAngle(float triggerAngle);
			void setTriggerAngleSpread(float triggerAngleSpread);
			void setTriggerMagnitudeRange(float triggerMagnitudeMin, float triggerMagnitudeMax);
			void setTriggerParams(float triggerAngle=DEFAULT_TRIGGER_ANGLE, float triggerAngleSpread=DEFAULT_TRIGGER_ANGLE_SPREAD, float triggerMagnitudeMin=DEFAULT_TRIGGER_MAGNITUDE_MIN, float triggerMagnitudeMax=DEFAULT_TRIGGER_MAGNITUDE_MAX);
		
			void drawTriggerAngleRange(int x, int y, int w, int h, float ratioX=1.0, float ratioY=1.0);
	};
	
protected:
	vector<AngleTrigger*> angleTriggers;
	int triggerDelay, fireTime;
	bool firing;
	ofRectangle* triggerArea;
	
	virtual void fire();
	virtual void ceaseFire();
	bool isReadyToFire();
	
	bool checkAngleTriggers(float angle, float magnitude);
	bool shouldTrigger();
	
public:
	MotionTrigger();
	~MotionTrigger();
	virtual void init(int cameraWidth=DEFAULT_CAMERA_WIDTH, int cameraHeight=DEFAULT_CAMERA_HEIGHT, int cameraID=DEFAULT_CAMERA_ID);
	
	bool isFiring();
	
	int addAngleTrigger(float triggerAngle=DEFAULT_TRIGGER_ANGLE, float triggerAngleSpread=DEFAULT_TRIGGER_ANGLE_SPREAD, float triggerMagnitudeMin=DEFAULT_TRIGGER_MAGNITUDE_MIN, float triggerMagnitudeMax=DEFAULT_TRIGGER_MAGNITUDE_MAX);
	void incrementAngleTriggerRanges(float increment);

	int getTriggerDelay();
	AngleTrigger* getAngleTrigger(int triggerID);
	float getAngleTriggerAngle(int triggerID);
	float getAngleTriggerAngleSpread(int triggerID);
	float getAngleTriggerMagnitudeMin(int triggerID);
	float getAngleTriggerMagnitudeMax(int triggerID);
	
	void setTriggerAngle(int triggerID, float triggerAngle);
	void setTriggerAngleSpread(int triggerID, float triggerAngleSpread);
	void setTriggerMagnitudeRange(int triggerID, float triggerMagnitudeMin, float triggerMagnitudeMax);
	
	void setAngleTriggerMagnitudeRanges(float magnitudeMin, float magnitudeMax);
	void setTriggerDelay(int triggerDelay);
	void setTriggerArea(ofRectangle* triggerArea);
	
	virtual void draw();
	void drawAngleTriggers(int x, int y, int w, int h, float ratioX=1.0, float ratioY=1.0);
	void drawOpticalFlowTriggers(int x, int y, int w, int h);
	virtual void update();
};

#endif
