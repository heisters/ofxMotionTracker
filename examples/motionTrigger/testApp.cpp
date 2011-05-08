#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	motionTracker.init();
	motionTracker.setOpticalFlowResolution(320, 240);
	motionTracker.setMirrorVideo(true);
	
	float angleSpread = 45;
	float magMin = 50;
	float magMax = 500;
	
	motionTracker.addAngleTrigger(0.0, angleSpread/2.0, magMin, magMax);
	motionTracker.addAngleTrigger(180.0, angleSpread/2.0,  magMin, magMax);
}

//--------------------------------------------------------------
void testApp::update(){
	motionTracker.update();
}

//--------------------------------------------------------------
void testApp::draw(){
	motionTracker.drawDebugVideo(0, 480, 320, 240);
	motionTracker.drawColor(0, 0, 640, 480);
	motionTracker.drawOpticalFlowTriggers(640, 0, 640, 480);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	motionTracker.keyPressed(key);
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

