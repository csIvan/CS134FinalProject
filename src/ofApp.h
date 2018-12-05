#pragma once

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "box.h"
#include "ray.h"
#include "Octree.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool  doPointSelection();
		void drawBox(const Box &box);
		Box meshBounds(const ofMesh &);
		void subDivideBox8(const Box &b, vector<Box> & boxList);
    void altRayDistance();

		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);

		ofEasyCam cam;
		ofxAssimpModelLoader mars, rover;
		ofLight light;
		Box boundingBox;
		vector<Box> level1, level2, level3;
	
		Octree octree;
		float timer;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		
		bool bRoverLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
        const float selectionRange = 4.0;
    
    // imported from midterm lander

    ofxAssimpModelLoader lander;
    ofImage backgroundImage;
    ofCamera *theCam = NULL;
    ofCamera topCam;
    
    const float SPD = 1;
    ofPoint posUpdater;
    
    ParticleSystem prover;
    Particle par;
    ofVec3f assigner;
	
    ImpulseRadialForce *exhaustForce;
    ParticleEmitter exhaust;
    
    TurbulenceForce *turb2;
    GravityForce *gravF;
    ImpulseRadialForce *radF;
    
    Thruster *thrust;
    
    
    TurbulenceForce *turbForce;
    bool bBackgroundLoaded = false;
    bool bLanderLoaded = false;
    
    
    
		
};
