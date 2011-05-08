#include "ofCircleSlice.h"

void ofCircleSlice(float x,float y, float radius, float lowAngle, float highAngle, bool closed, bool radians){
    if (!bSetupCircle) setupCircle();

    // use smoothness, if requested:
    if (bSmoothHinted && drawMode == OF_OUTLINE) startSmoothing();

    bool angleWrap = (lowAngle > highAngle); // are we doing the 0/360 wrap?

    if(!radians){
        lowAngle = ofDegToRad(lowAngle);
        highAngle = ofDegToRad(highAngle);
    }

    int res = numCirclePts;
    float angle = lowAngle;
    float angleRange = ((!angleWrap)?(highAngle - lowAngle):(M_TWO_PI - lowAngle + highAngle));
    float angleAdder = angleRange / (float)res;
    int k = 0;
    for (int i = 0; i < numCirclePts; i++){
        circlePtsScaled[k] = x + cos(angle) * radius;
        circlePtsScaled[k+1] = y - sin(angle) * radius;
        angle += angleAdder;
        k+=2;
    }

    // we draw the circle points ourself (vs. glDrawArrays) because it allows us to draw the center point, and have the triangles fan around it
    k = 0;
    glBegin((drawMode == OF_FILLED) ? GL_TRIANGLE_FAN : (closed?GL_LINE_LOOP:GL_LINE_STRIP));
    glVertex2f(x, y); // center vertex

    // now all the points around the circumference
    for (int i = 0; i < numCirclePts; i++){
        glVertex2f(circlePtsScaled[k], circlePtsScaled[k+1]);
        k+=2;
    }
    glEnd();

    // back to normal, if smoothness is on
    if (bSmoothHinted && drawMode == OF_OUTLINE) endSmoothing();
};
