/*
 *  MotionTracker.cpp
 *  openFrameworks
 *
 *  Created by Pat Long on 18/01/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "MotionTracker.h"

MotionTracker::MotionTracker(){
	this->opticalFlow = NULL;
	this->opticalFlowEnabled = false;
}

MotionTracker::~MotionTracker(){
	if(this->opticalFlow != NULL){
		delete this->opticalFlow;
		this->opticalFlow = NULL;
	}
}

void MotionTracker::init(int w, int h, int i){
	FrameDifferencer::init(w, h, i);
	
	colorCurDiff.allocate(camWidth, camHeight);
	colorPrev.allocate(camWidth, camHeight);
	greyCurDiff.allocate(camWidth, camHeight);
	greyPrev.allocate(camWidth, camHeight);
	
	colorCurDiff.set(0);
	colorPrev.set(0);
	greyCurDiff.set(0);
	greyPrev.set(0);
	
	this->setCamVelocityBlur();
}

void MotionTracker::reset(){
	FrameDifferencer::reset();
	
	if(colorCurDiff.bAllocated)
		colorCurDiff.set(0);
	if(colorPrev.bAllocated)
		colorPrev.set(0);
	if(greyCurDiff.bAllocated)
		greyCurDiff.set(0);
	if(greyPrev.bAllocated)
		greyPrev.set(0);
	if(this->opticalFlow != NULL){
		cvSetZero(this->opticalFlow->vel_x);
		cvSetZero(this->opticalFlow->vel_y);
	}
}

void MotionTracker::update(){
	FrameDifferencer::update();
	if(bHasNewFrame){
		if(this->opticalFlowEnabled && this->opticalFlow != NULL){
			// consecutive frame difference and optical flow
			if(frameCount > 5){		// dont do anything until we have enough in history				
				if(this->differenceMode == DIFFERENCE_MODE_GREY){
					ofxCvGrayscaleImage optFlowPrev = greyPrev;
					ofxCvGrayscaleImage optFlowBgDiff = greyBgDiff;
					
					optFlowPrev.resize(this->optFlowWidth, this->optFlowHeight);
					optFlowBgDiff.resize(this->optFlowWidth, this->optFlowHeight);
					
					opticalFlow->calc(optFlowPrev, optFlowBgDiff, 11);
					cvSmooth(opticalFlow->vel_x, opticalFlow->vel_x, CV_BLUR , camVelocityBlur);
					cvSmooth(opticalFlow->vel_y, opticalFlow->vel_y, CV_BLUR , camVelocityBlur);

					greyCurDiff.absDiff(greyPrev, greyBgDiff);			// curDiff is the difference between the last 2 frames
					greyCurDiff.threshold(camThreshold, CV_THRESH_TOZERO);		// chop dark areas
					greyCurDiff.blur(3);
				}
				else{
					ofxCvGrayscaleImage optFlowPrev = colorPrev;
					ofxCvGrayscaleImage optFlowBgDiff = colorBgDiff;
					
					optFlowPrev.resize(this->optFlowWidth, this->optFlowHeight);
					optFlowBgDiff.resize(this->optFlowWidth, this->optFlowHeight);
					
					opticalFlow->calc(optFlowPrev, optFlowBgDiff, 11);
					cvSmooth(opticalFlow->vel_x, opticalFlow->vel_x, CV_BLUR , camVelocityBlur);
					cvSmooth(opticalFlow->vel_y, opticalFlow->vel_y, CV_BLUR , camVelocityBlur);

					colorCurDiff.absDiff(greyPrev, greyBgDiff);			// curDiff is the difference between the last 2 frames
					colorCurDiff.threshold(camThreshold, CV_THRESH_TOZERO);		// chop dark areas
					colorCurDiff.blur(3);
				}
			}
			
			if(this->differenceMode == DIFFERENCE_MODE_GREY)
				greyPrev = greyBgDiff;
			else
				colorPrev = colorBgDiff;
		}
	}
}

void MotionTracker::drawDebugVideo(int x, int y, int w, int h){
	FrameDifferencer::drawDebugVideo(x, y, w, h);
	if(this->opticalFlowEnabled && this->opticalFlow != NULL)
		this->drawOpticalFlow(x+w*2, y, w, h, false);
}

void MotionTracker::drawOpticalFlow(int x, int y, int w, int h, bool drawBG){
	if(this->opticalFlow != NULL){
		if(drawBG){
			glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
			ofFill();
			ofRect(x, y, w, h);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else
			glColor3f(1.0f, 0.0f, 0.0f);
		glPushMatrix();
		glTranslatef(x, y, 0);
		glScalef((float)w / (float)optFlowWidth, (float)h / (float)optFlowHeight, 1.0f);
		this->opticalFlow->draw();
		glPopMatrix();
	}
}

void MotionTracker::drawOpticalFlowAverage(int x, int y, int w, int h, ofRectangle* averageBounds){
	float ratioX = (float)w / (float)this->optFlowWidth;
	float ratioY = (float)h / (float)this->optFlowHeight;
	float u, v;
	getVelAverageComponents(&u, &v, averageBounds);
	
	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	ofFill();
	ofRect(x, y, w, h);
	glColor3f(1.0f, 1.0f, 1.0f);
	ofCircle(x+w/2.0, y+h/2.0, 2);
	ofLine(x+w/2.0, y+h/2.0, x+w/2.0+u*ratioX/2.0, y+h/2.0+v*ratioY/2.0);
}

int	MotionTracker::getOptFlowCols(){
	return this->optFlowCols;
}

int MotionTracker::getOptFlowRows(){
	return this->optFlowRows;
}

float MotionTracker::getOptFlowColWidth(){
	return this->optFlowColWidth;
}

float MotionTracker::getOptFlowRowHeight(){
	return this->optFlowRowHeight;
}

void MotionTracker::getVelAtPixel(int x, int y, float *u, float *v) {
	if(this->opticalFlow != NULL){
		x = (float)x/(float)camWidth*(float)optFlowWidth; // normalize co-ords from camera size to optical flow size
		y = (float)y/(float)camHeight*(float)optFlowHeight;
		*u = cvGetReal2D( opticalFlow->vel_x, y, x );
		*v = cvGetReal2D( opticalFlow->vel_y, y, x );
	}
}

void MotionTracker::getVelAtNorm(float x, float y, float *u, float *v) {
	if(this->opticalFlow != NULL){
		int ix = x * optFlowWidth;
		int iy = y * optFlowHeight;
		if(ix<0) ix = 0; else if(ix>=optFlowWidth) ix = optFlowWidth - 1;
		if(iy<0) iy = 0; else if(iy>=optFlowHeight) iy = optFlowHeight - 1;
		*u = cvGetReal2D( opticalFlow->vel_x, iy, ix );
		*v = cvGetReal2D( opticalFlow->vel_y, iy, ix );
	}
}

void MotionTracker::getVelAverageComponents(float *u, float *v, ofRectangle* bounds, bool normalizedBounds){
	if(this->opticalFlow == NULL)
		return;
	
	bool cleanBounds = false;
	if(bounds == NULL){
		bounds = new ofRectangle(0, 0, optFlowWidth, optFlowHeight);
		cleanBounds = true;
	}
	
	if(!normalizedBounds){
		// normalize co-ords from camera size to optical flow size
		bounds->x = (float)bounds->x/(float)camWidth;
		bounds->y = (float)bounds->y/(float)camHeight;
		bounds->width = (float)bounds->width/(float)camWidth;
		bounds->height = (float)bounds->height/(float)camHeight;
	}
	// normalize the bounds to optical flow system
	bounds->x *= (float)optFlowWidth;
	bounds->y *= (float)optFlowHeight;
	bounds->width *= (float)optFlowWidth;
	bounds->height *= (float)optFlowHeight;
	
	// fix out of bounds
	if(bounds->x < 0){
		bounds->width += bounds->x;
		bounds->x = 0;
	}
	else if(bounds->x + bounds->width > optFlowWidth){
		bounds->width -= ((bounds->x + bounds->width) - optFlowWidth);
	}
	if(bounds->y < 0){
		bounds->height += bounds->y;
		bounds->y = 0;
	}
	else if(bounds->y + bounds->height > optFlowHeight){
		bounds->height -= ((bounds->y + bounds->height) - optFlowHeight);
	}
	
	/**	cout << "--bounds:norm--" << endl;
	 cout << "x:" << bounds->x << endl;
	 cout << "y:" << bounds->y << endl;
	 cout << "w:" << bounds->width << endl;
	 cout << "h:" << bounds->height << endl;
	 cout << "--------------------" << endl;*/
	
	*u = 0.0;
	*v = 0.0;
	for(int i=bounds->x; i < bounds->x+(int)bounds->width; i++){
		for(int j=bounds->y; j < bounds->y+(int)bounds->height; j++){
			*u += cvGetReal2D( opticalFlow->vel_x, j, i);
			*v += cvGetReal2D( opticalFlow->vel_y, j, i);
			//			cout << i << "," << j << "::" << *u << "," << *v << endl;
		}
	}
	
	*u /= 2.0 * (float)bounds->height; // effectively: u / (width*height) * width/2.0
	*v /= 2.0 * (float)bounds->width; // effectively: v / (width*height) * height/2.0
	
	if(cleanBounds)
		delete bounds;
}

void MotionTracker::updateOptFlowGrid(){
	this->optFlowCols = this->optFlowWidth;
	this->optFlowRows = this->optFlowHeight;
	this->optFlowColWidth = this->camWidth / this->optFlowCols;
	this->optFlowRowHeight = this->camHeight / this->optFlowRows;
}

void MotionTracker::getVelAverageAngleMag(float *angle, float *magnitude, ofRectangle* bounds, bool normalizedBounds){
	float u, v;
	this->getVelAverageComponents(&u, &v, bounds, normalizedBounds);
	*angle = tiRadiansToAngle(tiAngle(u, v));
	*magnitude = tiDistance(u, v);
}

bool MotionTracker::isOpticalFlowEnabled(){
	return this->opticalFlowEnabled;
}

void MotionTracker::setOpticalFlowEnabled(bool opticalFlowEnabled){
	this->opticalFlowEnabled = opticalFlowEnabled;
	if(opticalFlowEnabled && this->opticalFlow == NULL){
		this->opticalFlow = new ofxCvOpticalFlowLK();
		this->setOpticalFlowResolution();
	}
	else if(!opticalFlowEnabled && this->opticalFlow != NULL){
		delete this->opticalFlow;
		this->opticalFlow = NULL;
	}
}

void MotionTracker::setOpticalFlowResolution(int w, int h){
	if(!this->opticalFlowEnabled)
		this->setOpticalFlowEnabled(true);
	if(w <= 0.0 || w > this->camWidth)
		w = this->camWidth;
	if(h <= 0.0 || h > this->camHeight)
		h = this->camHeight;
	this->optFlowWidth = w;
	this->optFlowHeight = h;
	if(this->opticalFlow != NULL)
		this->opticalFlow->allocate(this->optFlowWidth, this->optFlowHeight);
	this->updateOptFlowGrid();
}

void MotionTracker::setCamVelocityBlur(float camVelocityBlur){
	this->camVelocityBlur = camVelocityBlur;
}
