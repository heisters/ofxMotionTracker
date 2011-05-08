#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	motionTracker.init(640, 480, 1);
	motionTracker.setMirrorVideo(true);
	motionTracker.setDifferenceMode(DIFFERENCE_MODE_COLOR);
	motionTracker.setCamThreshold(10);
}

//--------------------------------------------------------------
void testApp::update(){
	motionTracker.update();
}

//--------------------------------------------------------------
void testApp::draw(){
	motionTracker.drawDebugVideo(0, 480, 320, 240);
	motionTracker.drawColor(0, 0, 640, 480);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(!motionTracker.keyPressed(key)){
		if(key == 'g'){
			if(motionTracker.getDifferenceMode() == DIFFERENCE_MODE_GREY)
				motionTracker.setDifferenceMode(DIFFERENCE_MODE_COLOR);
			else
				motionTracker.setDifferenceMode(DIFFERENCE_MODE_GREY);
		}
	}
		
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

