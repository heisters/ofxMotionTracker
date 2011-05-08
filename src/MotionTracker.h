/*
 *  MotionTracker.h
 *  openFrameworks
 *
 *  Created by Pat Long on 18/01/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _MOTION_TRACKER
#define _MOTION_TRACKER

#include "FrameDifferencer.h"
#include "ofxCvOpticalFlowLK.h"
#include "ofxCvOpticalFlowBM.h"

#define DEFAULT_OPTICAL_FLOW_WIDTH	320
#define DEFAULT_OPTICAL_FLOW_HEIGHT	240
#define DEFAULT_CAM_VELOCITY_BLUR	15

class MotionTracker : public FrameDifferencer{
protected:
	ofxCvOpticalFlowLK*		opticalFlow;
	ofxCvGrayscaleImage 	colorCurDiff;
	ofxCvGrayscaleImage 	colorPrev;
	ofxCvGrayscaleImage 	greyCurDiff;
	ofxCvGrayscaleImage 	greyPrev;
	bool opticalFlowEnabled;
	
	int optFlowWidth, optFlowHeight; // pixel size of optical flow image (huge performance gains the smaller it is)
	int camVelocityBlur;
	
	int optFlowCols, optFlowRows;
	float optFlowColWidth, optFlowRowHeight;
	
	void updateOptFlowGrid();
	
public:
	MotionTracker();
	~MotionTracker();
	
	virtual void init(int w=DEFAULT_CAMERA_WIDTH, int h=DEFAULT_CAMERA_HEIGHT, int i=DEFAULT_CAMERA_ID);
	virtual void reset();
	
	virtual void update();
	
	virtual void drawDebugVideo(int x, int y, int w, int h);
	
	void drawOpticalFlow(int x, int y, int w, int h, bool drawBG=true);
	void drawOpticalFlowAverage(int x, int y, int w, int h, ofRectangle* averageBounds=NULL);
	
	int	getOptFlowCols();
	int getOptFlowRows();
	float getOptFlowColWidth();
	float getOptFlowRowHeight();
	void getVelAtPixel(int x, int y, float *u, float *v);			// takes coordinates in pixels 
	void getVelAtNorm(float x, float y, float *u, float *v);		// takes coordinates in normalized
	void getVelAverageComponents(float *u, float *v, ofRectangle* bounds=NULL, bool normalizedBounds=false);
	void getVelAverageAngleMag(float *angle, float *magnitude, ofRectangle* bounds=NULL, bool normalizedBounds=false);
	
	bool isOpticalFlowEnabled();
	
	void setOpticalFlowEnabled(bool opticalFlowEnabled);
	void setOpticalFlowResolution(int w=DEFAULT_OPTICAL_FLOW_WIDTH, int h=DEFAULT_OPTICAL_FLOW_HEIGHT);
	void setCamVelocityBlur(float camVelocityBlur=DEFAULT_CAM_VELOCITY_BLUR);
	
};

#endif
