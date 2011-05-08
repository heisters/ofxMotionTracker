/*
 *  FrameDifferencer.cpp
 *  openFrameworks
 *
 *  Created by Pat Long on 27/10/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "FrameDifferencer.h"

FrameDifferencer::FrameDifferencer(){
	this->contourFinderEnabled = false;
	this->contourFinder = NULL;
}

FrameDifferencer::~FrameDifferencer(){
	if(contourFinder != NULL)
		delete contourFinder;
}


void FrameDifferencer::init(int w, int h, int i){
	camWidth = w;
	camHeight = h;
	index = i;
	posX = index * camWidth;
	posY = 0;
	
	mirrorVideoHorizontal = false;
	mirrorVideoVertical = false;
	camThreshold = DEFAULT_CAM_THRESHOLD;
	camSilAlpha = DEFAULT_CAM_SIL_ALPHA;
	
	calibrationXMLFilename = DEFAULT_CALIBRATION_XML;
	
	vidGrabber.setDeviceID(i); // must set camera ID and other options before calling initGrabber
	vidGrabber.initGrabber(camWidth, camHeight, false); // no texture
	frameCount = 0;
	
	this->differenceMode = DEFAULT_DIFFERENCE_MODE;
	this->autoBackgroundDelay = -1;
	
	colorBg.allocate(camWidth, camHeight);
	colorBgCalibrated.allocate(camWidth, camHeight);
	colorNow.allocate(camWidth, camHeight);
	colorNowCalibrated.allocate(camWidth, camHeight);
	colorBgDiff.allocate(camWidth, camHeight);
	
	greyBg.allocate(camWidth, camHeight);
	greyBgCalibrated.allocate(camWidth, camHeight);
	greyNow.allocate(camWidth, camHeight);
	greyNowCalibrated.allocate(camWidth, camHeight);
	greyBgDiff.allocate(camWidth, camHeight);
	
	bLearnBG = true;
	bHasNewFrame = false;
	this->setMaxContourBlobs();
	this->setMinContourArea();
	this->setMaxContourArea();
	findContourHoles = false;
	blobCount = 0;
	
	calibrationPreview.width = camWidth/2.0;
	calibrationPreview.height = camHeight/2.0;
	calibrationPreview.x = ofGetWidth()/2.0 - calibrationPreview.width;
	calibrationPreview.y = (ofGetHeight() - calibrationPreview.height)/2.0;
	
	camSize.width = camWidth;
	camSize.height = camHeight;
	camSize.x = 0;
	camSize.y = 0;
	
	calibrationPoints[0].set(0.0, 0.0);
	calibrationPoints[1].set(1.0, 0.0);
	calibrationPoints[2].set(1.0, 1.0);
	calibrationPoints[3].set(0.0, 1.0);
	calibrationPointRadius = DEFAULT_CALIBRATION_POINT_RADIUS;
	calibrationMode = false;
	calibrationEnabled = true;
	
	distortionEnabled = false;
	for(int i=0; i < 8; i++)
		distortionCoefficients[i] = 0.0;
	
	reset();
}

bool FrameDifferencer::loadCalibrationSettings(string calibrationXML){
	if(calibrationXML == "")
		calibrationXML = this->calibrationXMLFilename;
	if(calibrationXML != this->calibrationXMLFilename)
		this->calibrationXMLFilename = calibrationXML;
	ofxXmlSettings xmlReader;
	stringstream logBuilder;
	bool result = xmlReader.loadFile(ofToDataPath(calibrationXML));
	if(!result){
		logBuilder << "Error reading frame differencer calibration xml file: " << calibrationXML << ".";
		ofLog(OF_LOG_ERROR, logBuilder.str());
		logBuilder.str("");
		return result;
	}
	
	ofPoint crop[4];
	if(xmlReader.tagExists("calibration")){
		xmlReader.pushTag("calibration");
		if(xmlReader.tagExists("camera")){
			xmlReader.pushTag("camera");
			if(xmlReader.tagExists("crop")){
				xmlReader.pushTag("crop");
				bool goodCrop = true;
				stringstream tagName;
				for(int i=0; i < 4 && goodCrop; i++){
					tagName << "pt" << i;
					if(xmlReader.tagExists(tagName.str())){
						xmlReader.pushTag(tagName.str());
						if(xmlReader.tagExists("x") && xmlReader.tagExists("y")){
							crop[i].x = xmlReader.getValue("x", 0.0f);
							crop[i].y = xmlReader.getValue("y", 0.0f);
						}
						else{
							goodCrop = false;
							logBuilder << "Invalid crop tag contents for: " << tagName.str();
							ofLog(OF_LOG_WARNING, logBuilder.str());
							logBuilder.str("");
						}
						xmlReader.popTag();
					}
					else{
						goodCrop = false;
						logBuilder << "Missing crop tag: " << tagName.str();
						ofLog(OF_LOG_WARNING, logBuilder.str());
						logBuilder.str("");
					}
					tagName.str("");
				}
				
				xmlReader.popTag();
				
				// done reading crop info...
				if(goodCrop){
					logBuilder << "Successfully read camera crop data:" << endl;
					logBuilder << "\t" << crop[0].x << "," << crop[0].y << "\t\t\t" << crop[1].x << "," << crop[1].y << endl;
					logBuilder << "\t" << crop[3].x << "," << crop[3].y << "\t\t\t" << crop[2].x << "," << crop[2].y;
					ofLog(OF_LOG_NOTICE, logBuilder.str());
					logBuilder.str("");
					
					// use the settings on our camera image
					for(int i=0; i < 4; i++)
						this->calibrationPoints[i].set(crop[i].x, crop[i].y);
				}
				else{
					logBuilder << "Error reading camera crop data";
					ofLog(OF_LOG_ERROR, logBuilder.str());
					logBuilder.str("");
					result = false;
				}
			}
			
			if(xmlReader.tagExists("distortion")){
				xmlReader.pushTag("distortion");
				bool goodDistort = true;
				
				bool distortEnabled = true;
				if(xmlReader.tagExists("enabled")){
					distortEnabled = (tiStrToLower(xmlReader.getValue("enabled", "true")) == "true");
				}
				
				float distorCoeffs[8];
				for(int i=0; i < 8 && goodDistort; i++){
					stringstream tagName;
					tagName << "coef" << i;
					
					if(xmlReader.tagExists(tagName.str())){
						distorCoeffs[i] = xmlReader.getValue(tagName.str(), 0.0f);
					}
					else{
						goodDistort = false;
						logBuilder << "Missing distortion tag: " << tagName.str();
						ofLog(OF_LOG_WARNING, logBuilder.str());
						logBuilder.str("");
					}
					
					tagName.str("");
				}
				
				xmlReader.popTag();
				
				// done reading distortion data
				if(goodDistort){
					logBuilder << "Successfully read camera distortion data:" << endl;
					logBuilder << "\tDistortion: " << (distortEnabled?"enabled":"disabled");
					for(int i=0; i < 8; i++)
						logBuilder << endl << "\tcoefficient[" << i << "] " << distorCoeffs[i];
					ofLog(OF_LOG_NOTICE, logBuilder.str());
					logBuilder.str("");
					
					// use the settings on our camera image
					this->distortionEnabled = distortEnabled;
					for(int i=0; i < 8; i++)
						this->distortionCoefficients[i] = distorCoeffs[i];
				}
				else{
					logBuilder << "Error reading camera distortion data";
					ofLog(OF_LOG_ERROR, logBuilder.str());
					logBuilder.str("");
					result = false;
				}
			}
			
			xmlReader.popTag();
		}
		else
			result = false;
		
		xmlReader.popTag();
	}
	else
		result = false;
	
	return result;
}

bool FrameDifferencer::saveCalibrationSettings(string calibrationXML){
	if(calibrationXML == "")
		calibrationXML = this->calibrationXMLFilename;
	if(calibrationXML != this->calibrationXMLFilename)
		this->calibrationXMLFilename = calibrationXML;
	ofxXmlSettings xmlWriter;
	stringstream logBuilder;
	bool result = xmlWriter.loadFile(ofToDataPath(calibrationXML));
	if(!result){
		logBuilder << "Error loading xml file for writing: " << calibrationXML << ".";
		ofLog(OF_LOG_ERROR, logBuilder.str());
		logBuilder.str("");
		return result;
	}
	
	stringstream tagBuilder;
	
	if(!xmlWriter.tagExists("calibration"))
		xmlWriter.addTag("calibration");
	xmlWriter.pushTag("calibration");
	
	if(!xmlWriter.tagExists("camera"))
		xmlWriter.addTag("camera");
	xmlWriter.pushTag("camera");
	
	if(!xmlWriter.tagExists("crop"))
		xmlWriter.addTag("crop");
	xmlWriter.pushTag("crop");

	for(int i=0; i < 4; i++){
		tagBuilder << "pt" << i;
		if(!xmlWriter.tagExists(tagBuilder.str()))
			xmlWriter.addTag(tagBuilder.str());
		xmlWriter.pushTag(tagBuilder.str());
		
		if(!xmlWriter.tagExists("x"))
			xmlWriter.addTag("x");
		xmlWriter.setValue("x", this->calibrationPoints[i].x);
		
		if(!xmlWriter.tagExists("y"))
			xmlWriter.addTag("y");
		xmlWriter.setValue("y", this->calibrationPoints[i].y);
		
		xmlWriter.popTag(); // pt<i>
		
		tagBuilder.str("");
	}
	xmlWriter.popTag(); // crop
	
	
	if(!xmlWriter.tagExists("distortion"))
		xmlWriter.addTag("distortion");
	xmlWriter.pushTag("distortion");
	
	if(!xmlWriter.tagExists("enabled"))
		xmlWriter.addTag("enabled");
	xmlWriter.setValue("enabled", (this->distortionEnabled?"true":"false"));
	
	for(int i=0; i < 8; i++){
		tagBuilder << "coef" << i;
		
		if(!xmlWriter.tagExists(tagBuilder.str()))
			xmlWriter.addTag(tagBuilder.str());
		xmlWriter.setValue(tagBuilder.str(), this->distortionCoefficients[i]);
		
		tagBuilder.str("");
	}
	xmlWriter.popTag(); // distortion
	
	xmlWriter.popTag(); // camera
	xmlWriter.popTag(); // calibration
	
	if(result){
		xmlWriter.saveFile(ofToDataPath(calibrationXML));
		logBuilder << "successfully saved calibration settings to: " << calibrationXML << endl;
		ofLog(OF_LOG_NOTICE, logBuilder.str());
		logBuilder.str("");
	}
	
	return result;
}

void FrameDifferencer::reset() {
	colorBg.set(0);
	colorBgCalibrated.set(0);
	colorNow.set(0);
	colorNowCalibrated.set(0);
	colorBgDiff.set(0);
	
	greyBg.set(0);
	greyBgCalibrated.set(0);
	greyNow.set(0);
	greyNowCalibrated.set(0);
	greyBgDiff.set(0);
}


//--------------------------------------------------------------
void FrameDifferencer::update(){
	//	printf("starting MotionTracker %i update \n", index);
	vidGrabber.grabFrame();
	
	if(bHasNewFrame = vidGrabber.isFrameNew()){
		colorNow.setFromPixels(vidGrabber.getPixels(), camWidth,camHeight);
		colorNow.mirror(mirrorVideoVertical, mirrorVideoHorizontal);
		
		if(distortionEnabled){
			colorNow.undistort(distortionCoefficients[0], distortionCoefficients[1],
							   distortionCoefficients[2], distortionCoefficients[3],
							   distortionCoefficients[4], distortionCoefficients[5],
							   distortionCoefficients[6], distortionCoefficients[7]);
		}
		
		if(calibrationEnabled){
			colorNowCalibrated = colorNow;
			colorNowCalibrated.warpPerspective(mapPoint(calibrationPoints[0], camSize, false), mapPoint(calibrationPoints[1], camSize, false), mapPoint(calibrationPoints[2], camSize, false), mapPoint(calibrationPoints[3], camSize, false));
		}
		
		greyNow.setFromColorImage(colorNow);			// make color image grey
		greyNowCalibrated.setFromColorImage(colorNowCalibrated);
		
		int cTime = ofGetElapsedTimeMillis();
		if(this->shouldRecaptureBackground(cTime))
			bLearnBG = true;
		
		if (frameCount > 5 && bLearnBG == true) {							// save background if nessecary
			greyBg = greyNow;
			greyBgCalibrated = greyNowCalibrated;
			colorBg = colorNow;
			colorBgCalibrated = colorNowCalibrated;
			lastBackgroundCapture = cTime;
			bLearnBG = false;
		}
		
		// bg difference
		if(this->differenceMode == DIFFERENCE_MODE_GREY){
			if(calibrationEnabled)
				greyBgDiff.absDiff(greyBgCalibrated, greyNowCalibrated);
			else
				greyBgDiff.absDiff(greyBg, greyNow);			// subtract background from it
			greyBgDiff.threshold(camThreshold); //, CV_THRESH_TOZERO);		// chop dark areas		
			greyBgDiff.blur(3);
		}
		else{
			if(calibrationEnabled)
				calculateColourDifference(&colorBgCalibrated, &colorNowCalibrated, &colorBgDiff, camThreshold);
			else
				calculateColourDifference(&colorBg, &colorNow, &colorBgDiff, camThreshold);
			colorBgDiff.blur(6);
		}
		
		if(this->contourFinderEnabled){
			this->blobCount = this->findContours();
		}
		
		frameCount++;
	} 	
	//	printf("ending MotionTracker %i update \n", index);
}

void FrameDifferencer::calculateColourDifference(ofxCvColorImage* img1, ofxCvColorImage* img2, ofxCvGrayscaleImage* result, int threshold){
	int width = result->width;
	int height = result->height;
	int bpp = 3;
	unsigned char* pixels1 = img1->getPixels();
	unsigned char* pixels2 = img2->getPixels();
	unsigned char* resPixels = new unsigned char[width * height]; // single channel - b & w
	bool pixelDiff;
	
	for(int i=0; i < height; i++){
		for(int j=0; j < width; j++){
			pixelDiff = false;
			
			for(int b=0; b < bpp; b++){
				int diff = pixels1[(i*width+j)*bpp+b] - pixels2[(i*width+j)*bpp+b];
				diff = (diff < 0)?-diff:diff;
				if(diff > threshold){
					pixelDiff = true;
					break;
				}
			}
			resPixels[i*width+j] = (pixelDiff)?255:0;
		}
	}
	result->setFromPixels(resPixels, width, height);
	delete resPixels;
}

bool FrameDifferencer::shouldRecaptureBackground(int cTime){
	if(cTime == -1)
		cTime = ofGetElapsedTimeMillis();
	if(this->autoBackgroundDelay != -1 && (cTime - lastBackgroundCapture) >= this->autoBackgroundDelay)
		return true;
	return false;
}

void FrameDifferencer::setPosition(int i) {
	index = i;
	posX = index * camWidth;
}

void FrameDifferencer::setMirrorVideo(bool horizontal, bool vertical){
	this->mirrorVideoHorizontal = horizontal;
	this->mirrorVideoVertical = vertical;
}

void FrameDifferencer::setCamThreshold(int camThreshold){
	if(camThreshold < 0) camThreshold = 0;
	else if(camThreshold > 255) camThreshold = 255;
	this->camThreshold = camThreshold;
}

void FrameDifferencer::setCamSilAlpha(float camSilAlpha){
	this->camSilAlpha = camSilAlpha;
}

void FrameDifferencer::setDifferenceMode(int differenceMode){
	this->differenceMode = differenceMode;
}

void FrameDifferencer::setAutoBackgroundDelay(int autoBackgroundDelay){
	this->autoBackgroundDelay = autoBackgroundDelay;
}

void FrameDifferencer::setContourFinderEnabled(bool contourFinderEnabled){
	this->contourFinderEnabled = contourFinderEnabled;
	if(contourFinderEnabled && this->contourFinder == NULL){
		this->contourFinder = new ofxCvContourFinder();
		this->setMaxContourBlobs();
	}
	if(!contourFinderEnabled){
		if(this->maxContourBlobs > 0)
			this->setMaxContourBlobs(0);
		if(this->contourFinder != NULL){
			delete this->contourFinder;
			this->contourFinder = NULL;
		}
	}
}

void FrameDifferencer::setMaxContourBlobs(int maxContourBlobs){
	this->maxContourBlobs = maxContourBlobs;
	if(maxContourBlobs > 0 && !this->contourFinderEnabled)
		this->setContourFinderEnabled(true);
	else if(maxContourBlobs <= 0 && this->contourFinderEnabled)
		this->setContourFinderEnabled(false);
		
}

void FrameDifferencer::setMinContourArea(float minContourArea){
	this->minContourArea = minContourArea;
}

void FrameDifferencer::setMaxContourArea(float maxContourArea){
	if(maxContourArea == -1)
		maxContourArea = camWidth*camHeight;
	this->maxContourArea = maxContourArea;
}

void FrameDifferencer::setFindContourHoles(bool findContourHoles){
	this->findContourHoles = findContourHoles;
}

int FrameDifferencer::getCamWidth(){
	return this->camWidth;
}

int FrameDifferencer::getCamHeight(){
	return this->camHeight;
}

int FrameDifferencer::getCamThreshold(){
	return this->camThreshold;
}

unsigned char* FrameDifferencer::getColorPixels(){
	return this->vidGrabber.getPixels();
}

unsigned char* FrameDifferencer::getBGDiffPixels(){
	return this->greyBgDiff.getPixels();
}

int FrameDifferencer::getDifferenceMode(){
	return this->differenceMode;
}

int FrameDifferencer::getAutoBackgroundDelay(){
	return this->autoBackgroundDelay;
}

bool FrameDifferencer::hasBlobs(){
	return (this->blobCount > 0);
}

bool FrameDifferencer::isCalibrating(){
	return this->calibrationMode;
}

bool FrameDifferencer::isContourFinderEnabled(){
	return this->contourFinderEnabled;
}

int FrameDifferencer::findContours(){
	// if we have a background
	if(!bLearnBG && maxContourBlobs > 0 && this->contourFinder != NULL){
		return this->contourFinder->findContours(((this->differenceMode == DIFFERENCE_MODE_COLOR)?colorBgDiff:greyBgDiff), minContourArea, maxContourArea, maxContourBlobs, findContourHoles);	
	}
	return 0;
}

//--------------------------------------------------------------
void FrameDifferencer::drawCalibration(bool withBackground){
	if(!this->isCalibrating())
		return;
	
	float maxX = ofGetWidth();
	float maxY = ofGetHeight();
	
	if(withBackground){
		ofSetColor(0, 127, 64);
		ofRect(0, 0, maxX, maxY);
	}
	
	ofSetColor(0, 255, 0);
	ofDrawBitmapString("Calibrating!", calibrationPreview.x, calibrationPreview.y-20);
	
	ofSetColor(255, 255, 255);
	
	this->drawColor(calibrationPreview.x, calibrationPreview.y, calibrationPreview.width, calibrationPreview.height);
	this->drawColorCalibrated(calibrationPreview.x+calibrationPreview.width+10, calibrationPreview.y, calibrationPreview.width, calibrationPreview.height);
	
	ofNoFill();
	ofRect(calibrationPreview.x, calibrationPreview.y, calibrationPreview.width, calibrationPreview.height);
	
	ofSetColor(0, 255, 255);
	
	ofLine(mapX(calibrationPoints[0], calibrationPreview), mapY(calibrationPoints[0], calibrationPreview), mapX(calibrationPoints[1], calibrationPreview), mapY(calibrationPoints[1], calibrationPreview));
	ofLine(mapX(calibrationPoints[1], calibrationPreview), mapY(calibrationPoints[1], calibrationPreview), mapX(calibrationPoints[2], calibrationPreview), mapY(calibrationPoints[2], calibrationPreview));
	ofLine(mapX(calibrationPoints[2], calibrationPreview), mapY(calibrationPoints[2], calibrationPreview), mapX(calibrationPoints[3], calibrationPreview), mapY(calibrationPoints[3], calibrationPreview));
	ofLine(mapX(calibrationPoints[3], calibrationPreview), mapY(calibrationPoints[3], calibrationPreview), mapX(calibrationPoints[0], calibrationPreview), mapY(calibrationPoints[0], calibrationPreview));
	
	ofFill();
	for(int i=0; i < 4; i++){
		if(calibrationPoints[i].active)
			ofSetColor(0, 255, 0);
		else
			ofSetColor(0, 255, 255);
		ofCircle(mapX(calibrationPoints[i], calibrationPreview), mapY(calibrationPoints[i], calibrationPreview), calibrationPointRadius);
	}
	
	ofSetColor(255, 255, 255);
}

//--------------------------------------------------------------
void FrameDifferencer::drawDebugVideo(int x, int y, int w, int h){
	//		printf("drawing MOTION TRACKER %i \n", index);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	x -= w;
	
	if(this->differenceMode == DIFFERENCE_MODE_COLOR){
		this->drawColorBGCalibrated(x += w, y, w, h);
		this->drawColorCalibrated(x += w, y, w, h);
		this->drawColorBGDiff(x += w, y, w, h);
	}
	else{
		this->drawGreyBGCalibrated(x += w, y, w, h);
		this->drawGreyCalibrated(x += w, y, w, h);
		this->drawGreyBGDiff(x += w, y, w, h);
	}
	if(this->contourFinderEnabled)
		this->drawContours(x, y, w, h);
}

void FrameDifferencer::drawBGDiff(int x, int y, int w, int h){
	if(this->differenceMode == DIFFERENCE_MODE_COLOR)
		this->drawColorBGDiff(x, y, w, h);
	else
		this->drawGreyBGDiff(x, y, w, h);
}


void FrameDifferencer::drawColorBG(int x, int y, int w, int h){
	//	glColor3f(camSilAlpha, camSilAlpha, camSilAlpha);
	colorBg.draw(x, y, w, h);
}

void FrameDifferencer::drawColorBGCalibrated(int x, int y, int w, int h){
	colorBgCalibrated.draw(x, y, w, h);
}

void FrameDifferencer::drawColor(int x, int y, int w, int h){
	colorNow.draw(x, y, w, h);
}

void FrameDifferencer::drawColorCalibrated(int x, int y, int w, int h){
	colorNowCalibrated.draw(x, y, w, h);
}

void FrameDifferencer::drawColorBGDiff(int x, int y, int w, int h){
	colorBgDiff.draw(x, y, w, h);
}


void FrameDifferencer::drawGreyBG(int x, int y, int w, int h){
	greyBg.draw(x, y, w, h);
}

void FrameDifferencer::drawGreyBGCalibrated(int x, int y, int w, int h){
	greyBgCalibrated.draw(x, y, w, h);
}

void FrameDifferencer::drawGrey(int x, int y, int w, int h){
	greyNow.draw(x, y, w, h);
}

void FrameDifferencer::drawGreyCalibrated(int x, int y, int w, int h){
	greyNowCalibrated.draw(x, y, w, h);
}

void FrameDifferencer::drawGreyBGDiff(int x, int y, int w, int h){
	greyBgDiff.draw(x, y, w, h);
}

void FrameDifferencer::drawContours(int x, int y, int w, int h){
	ofSetColor(255, 255, 0);
	if(this->contourFinder != NULL)
		this->contourFinder->draw(x, y, w, h);
	ofSetColor(255, 255, 255);
}


//--------------------------------------------------------------
bool FrameDifferencer::keyPressed(int key){ 
	switch (key){
		case 'c':
			if(this->calibrationMode)
				this->saveCalibrationSettings();
			this->calibrationMode = !this->calibrationMode;
			return true;
			break;
			
		case 'v':
		case 'V':
			if(ofGetWindowMode() == OF_WINDOW) vidGrabber.videoSettings();
			return true;
			break;

		case ' ':
			bLearnBG = true;
			return true;
			break;
	}
	return false;
}

bool FrameDifferencer::isCalibrationPointHovered(int pointIdx, int x, int y){	
	return (x >= (mapX(calibrationPoints[pointIdx], calibrationPreview)-calibrationPointRadius) && x <= (mapX(calibrationPoints[pointIdx], calibrationPreview)+calibrationPointRadius)
			&& y >= (mapY(calibrationPoints[pointIdx], calibrationPreview)-calibrationPointRadius) && y <= (mapY(calibrationPoints[pointIdx], calibrationPreview)+calibrationPointRadius));
}

//--------------------------------------------------------------
bool FrameDifferencer::mouseMoved(int x, int y ){
	if(!this->isCalibrating())
		return false;
}

//--------------------------------------------------------------
bool FrameDifferencer::mouseDragged(int x, int y, int button){
	if(!this->isCalibrating())
		return false;
	
	for(int i=0; i < 4; i++){
		if(calibrationPoints[i].active){ //this->isCalibrationPointHovered(i, x, y)){
			float newX = normalizeX(x, calibrationPreview);
			float newY = normalizeY(y, calibrationPreview);
			if(newX > 1.0)
				newX = 1.0;
			else if(newX < 0.0)
				newX = 0.0;
			if(newY > 1.0)
				newY = 1.0;
			else if(newY < 0.0)
				newY = 0.0;			
			this->calibrationPoints[i].set(newX, newY);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
bool FrameDifferencer::mousePressed(int x, int y, int button){
	if(!this->isCalibrating())
		return false;
	bool result = false;
	for(int i=0; i < 4; i++){
		if(this->isCalibrationPointHovered(i, x, y)){
			calibrationPoints[i].active = true;
			result = true;
		}
		else
			calibrationPoints[i].active = false;
	}
	return result;
}

//--------------------------------------------------------------
bool FrameDifferencer::mouseReleased(int x, int y, int button){
	if(!this->isCalibrating())
		return false;
	bool result = false;
	for(int i=0; i < 4; i++){
		if(calibrationPoints[i].active) result = true;
		calibrationPoints[i].active = false;
	}
	return result;
}
