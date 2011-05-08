/*
 *  FrameDifferencer.h
 *  openFrameworks
 *
 *  Created by Pat Long on 27/10/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _FRAME_DIFFERENCER
#define _FRAME_DIFFERENCER

#include "ofMain.h"
#include "ofxCvMain.h"
#include "ofxTI_Utils.h"
#include "ofxXmlSettings.h"

#define DIFFERENCE_MODE_GREY	0
#define DIFFERENCE_MODE_COLOR	1
#define DEFAULT_DIFFERENCE_MODE DIFFERENCE_MODE_GREY

#define DEFAULT_CAMERA_ID		0
#define DEFAULT_CAMERA_WIDTH	640
#define DEFAULT_CAMERA_HEIGHT	480

#define DEFAULT_CAM_THRESHOLD	30
#define DEFAULT_CAM_SIL_ALPHA	1.0f

#define DEFAULT_CONTOUR_BLOB_MAX	10
#define DEFAULT_CONTOUR_MIN_AREA	20
#define DEFAULT_CONTOUR_MAX_AREA	-1

#define DEFAULT_CALIBRATION_XML		"calibration.xml"

#define DEFAULT_CALIBRATION_POINT_RADIUS	5.0

class CalibrationPoint : public ofPoint{
public:
	bool active;
	
	CalibrationPoint(float x=0, float y=0):ofPoint(x, y){ this->active = false; };
};

class FrameDifferencer{
protected:
	ofVideoGrabber			vidGrabber;
	ofxCvColorImage			colorBg;
	ofxCvColorImage			colorBgCalibrated;
	ofxCvColorImage			colorNow;
	ofxCvColorImage			colorNowCalibrated;
	ofxCvGrayscaleImage		colorBgDiff;
	
	ofxCvGrayscaleImage 	greyBg;
	ofxCvGrayscaleImage 	greyBgCalibrated;
	ofxCvGrayscaleImage 	greyNow;
	ofxCvGrayscaleImage 	greyNowCalibrated;
	ofxCvGrayscaleImage 	greyBgDiff;
	
	int differenceMode;
	
	bool					bLearnBG, mirrorVideoHorizontal, mirrorVideoVertical;
	int						camWidth, camHeight;		// pixel size of camera
	int						posX, posY;					// pixel position of this camera in the global composition
	int						camThreshold;
	float					camSilAlpha;
	int						frameCount;
	int						autoBackgroundDelay, lastBackgroundCapture;
	
	bool					contourFinderEnabled;
	int						maxContourBlobs;
	float					minContourArea, maxContourArea;
	bool					findContourHoles;
	int						blobCount;
	
	string calibrationXMLFilename;
	
	ofRectangle calibrationPreview;
	ofRectangle camSize;
	CalibrationPoint calibrationPoints[4];
	float calibrationPointRadius;
	bool calibrationMode;
	bool calibrationEnabled;
	
	float distortionCoefficients[8];
	bool distortionEnabled;
	
	void calculateColourDifference(ofxCvColorImage* img1, ofxCvColorImage* img2, ofxCvGrayscaleImage* result, int threshold);
	
	bool isCalibrationPointHovered(int pointIdx, int x, int y);
	
	bool shouldRecaptureBackground(int cTime=-1);
	
public:
	bool					bHasNewFrame;
	int						index;
	ofxCvContourFinder*		contourFinder;
	
	FrameDifferencer();
	~FrameDifferencer();
	
	virtual void init(int w=DEFAULT_CAMERA_WIDTH, int h=DEFAULT_CAMERA_HEIGHT, int i=DEFAULT_CAMERA_ID);
	virtual void update();
	virtual void reset();
	bool hasNewFrame() { return bHasNewFrame; }
	
	bool loadCalibrationSettings(string calibrationXML="");
	bool saveCalibrationSettings(string calibrationXML="");
	
	void setPosition(int i);
	void setMirrorVideo(bool horizontal, bool vertical=false);
	void setCamThreshold(int camThreshold);
	void setCamSilAlpha(float camSilAlpha);
	void setDifferenceMode(int differenceMode);
	void setAutoBackgroundDelay(int autoBackgroundDelay);
	
	void setContourFinderEnabled(bool contourFinderEnabled);
	void setMaxContourBlobs(int maxContourBlobs=DEFAULT_CONTOUR_BLOB_MAX);
	void setMinContourArea(float minContourArea=DEFAULT_CONTOUR_MIN_AREA);
	void setMaxContourArea(float maxContourArea=DEFAULT_CONTOUR_MAX_AREA);
	void setFindContourHoles(bool findContourHoles);
	
	int getCamWidth();
	int getCamHeight();
	int getCamThreshold();
	unsigned char* getColorPixels();
	unsigned char* getBGDiffPixels();
	int getDifferenceMode();
	int getAutoBackgroundDelay();
	bool isCalibrating();
	bool isContourFinderEnabled();
	
	bool hasBlobs();
	int findContours();
	
	void drawCalibration(bool withBackground=true);
	
	void drawColorBG(int x, int y, int w, int h);
	void drawColorBGCalibrated(int x, int y, int w, int h);
	void drawColor(int x, int y, int w, int h);
	void drawColorCalibrated(int x, int y, int w, int h);
	void drawColorBGDiff(int x, int y, int w, int h);
	
	void drawGreyBG(int x, int y, int w, int h);
	void drawGreyBGCalibrated(int x, int y, int w, int h);
	void drawGrey(int x, int y, int w, int h);
	void drawGreyCalibrated(int x, int y, int w, int h);
	void drawGreyBGDiff(int x, int y, int w, int h);
	
	void drawBGDiff(int x, int y, int w, int h);
	virtual void drawDebugVideo(int x, int y, int w, int h);
	
	void drawContours(int x, int y, int w, int h);
	
	bool keyPressed(int key);
	bool mouseMoved(int x, int y );
	bool mouseDragged(int x, int y, int button);
	bool mousePressed(int x, int y, int button);
	bool mouseReleased(int x, int y, int button);
	
};

#endif
