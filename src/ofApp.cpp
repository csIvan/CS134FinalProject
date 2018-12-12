
//--------------------------------------------------------------
//  Student Name:   < Ivan Hernandez , Linhson Bui>
//  Date: <12/13/2018>
//
//  Space Ship Spotted - CS134 Final Project
//  In this game you are exploring an unknown land
//      as an alien in a flying saucer
//  You want to find and abduct any cows you can find
//  Unforunately for you, there is no source of water in this land
//  That means no form of life, that includes cows
//
//  We are using octrees to determine an intersection
//  and phyics to resolve the collision
//  Cameras are set up to track the ship, see a bottom view from the ship
//      see a side view from the ship, and a free view of the ship
//
//  Use the arrows keys to move around
//  keys 1-5 change the camera angles
//  p toggles the points of collision we are testing for intersection
//  c enables movement of the camera
//  w toggles the wireframe of the terrain
//
//
//

#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bRoverLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	cam.setDistance(30);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
    theCam = &cam;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	mars.loadModel("geo/terrain_v2.obj");
	mars.setScaleNormalization(false);

	boundingBox = meshBounds(mars.getMesh(0));
    
    altitude = 0;
	


	// Create the Octree by passing in the mesh and the amount of levels
	octree.create(mars.getMesh(0), 8);

	// To appropriate the rotation of the mesh
	mars.setRotation(0, 180, 0, 0, 1);
    
    
    // Rover setup
    
    // load BG image
    //
    bBackgroundLoaded = backgroundImage.load("images/starry_bg.jpg");
    backgroundImage.resize(ofGetWidth(), ofGetHeight());
    
    // load lander model
    //
    if (lander.loadModel("geo/spaceShip.obj")) {
        lander.setScaleNormalization(false);
        lander.setScale(.25, .25, .25);
        lander.setRotation(0, -180, 1, 0, 0);
        lander.setPosition(0,0,0);
        
        bLanderLoaded = true;
        
        
    }
    else {
        cout << "Error: Can't load model" << "geo/spaceShip.obj" << endl;
        ofExit(0);
    }
    
    trackingCam.setFov(90);
    trackingCam.setPosition(30, 10, 0);
    sideCam.setFov(90);
    downCam.setFov(90);
    downCam.lookAt(glm::vec3(0, 0, 0));
    
    
    // particle System with 1 particle
    prover.add(par);
    
    //turbulence force
    turbForce = new TurbulenceForce(ofVec3f(-0.7,-0.7,-0.7),ofVec3f(0.7,0.7,0.7));
    
    // setting up the vectors that move the particle
    thrust = new Thruster(ofVec3f(0,0,0));
    
    // exhaust particles
    turb2 = new TurbulenceForce(ofVec3f(-2,-2,-2),ofVec3f(2,2,2));
    gravF = new GravityForce(ofVec3f(0,-.09,0));
    radF = new ImpulseRadialForce(100.0);
    resForce = new ImpulseForce();

    exhaustForce = new ImpulseRadialForce(50);
    exhaust.sys->addForce(exhaustForce);

    //set the emitter's variables
    exhaust.setVelocity(ofVec3f(0,-4,0));
    exhaust.setEmitterType(DiskEmitter);
    exhaust.setGroupSize(2);
    exhaust.setLifespan(.8);
    exhaust.setParticleRadius(.01);
    exhaust.setRate(35);
    
    
    prover.addForce(turbForce);
    prover.addForce(thrust);
    prover.addForce(resForce);
    prover.addForce(gravF);
    
    shipBox = Box(Vector3(-1,-1,-1),Vector3(1,1,1));
    
    prover.particles[0].position = ofVec3f(-20,5,0);
    cam.setPosition(ofVec3f(-20,0,0));
    
    
    //setup for ufo audio
    if(ufo.load("ufo.mp3"))
        soundSet = true;
    
    //setup for theme audio
    if(theme.load("space_theme.mp3"))
        tmSet = true;
    
    if(tmSet){
        theme.setVolume(.5);
        theme.play();
    }
    
    //light setup
    keyLight.setup();
    keyLight.enable();
    keyLight.setAreaLight(1, 1);
    keyLight.setAmbientColor(ofFloatColor(1, 1, 1));
    keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
    keyLight.setSpecularColor(ofFloatColor(1, 1, 1));
    
    keyLight.rotate(45, ofVec3f(0, 1, 0));
    keyLight.rotate(45, ofVec3f(1, 0, 0));
    keyLight.setPosition(glm::vec3(0, 200, 0));
    keyLight.lookAt(glm::vec3(0, 0, 0));
    
}

//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    prover.update();
    
    exhaust.setPosition(ofVec3f(prover.particles[0].position.x, prover.particles[0].position.y + .25, prover.particles[0].position.z));
    exhaust.update();
    assigner = prover.particles[0].position;
    lander.setPosition(assigner.x , assigner.y , assigner.z);
    altRayDistance();
    
    //Tracking Camera
    trackingCam.setPosition(glm::vec3(lander.getPosition().x + 10, lander.getPosition().y + 2, lander.getPosition().z + 10));
    trackingCam.lookAt(glm::vec3(lander.getPosition().x, lander.getPosition().y + 2, lander.getPosition().z));
    
    
    //side view camera
    sideCam.setPosition(glm::vec3(lander.getPosition().x - 1, lander.getPosition().y + 4, lander.getPosition().z));
    sideCam.lookAt(glm::vec3(glm::vec3(lander.getPosition().x + .30, lander.getPosition().y + 4, lander.getPosition().z)));
    
    
    //ground view camera
    downCam.setPosition(glm::vec3(lander.getPosition().x, lander.getPosition().y + 5, lander.getPosition().z));
    downCam.lookAt(glm::vec3(glm::vec3(prover.particles[0].position.x, -1*(prover.particles[0].position.y + .30), prover.particles[0].position.z)));
    
    
    // updates the collision box of the space ship
    shipBox.parameters[0] = Vector3(assigner.x , assigner.y , assigner.z) + Vector3(-1.7,0,-1.7);
    shipBox.parameters[1] =Vector3(assigner.x , assigner.y , assigner.z) + Vector3(1.7,1.75,1.7);
    // dimension of our shipBox is 3.4 x 1.75. 3.4
    // dont know how to get dimensions from model directly so box is hard coded

    
    // looks for collisions every frame
    collisionDect();
    
    
    // if there is a collision, turn off the turbulence force
    if (colDetected == true){
        turbForce->set(ofVec3f(0,0,0), ofVec3f(0,0,0));
        resCollision();
    }

    
}
//--------------------------------------------------------------
void ofApp::draw(){
    
    // draws the background if one is loaded
    if (bBackgroundLoaded) {
        ofPushMatrix();
        ofDisableDepthTest();
        backgroundImage.draw(0, 0);
        ofEnableDepthTest();
        ofPopMatrix();
    }
    
    cam.begin();
    theCam->begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bRoverLoaded) {
			rover.drawWireframe();
			if (!bTerrainSelected) drawAxis(rover.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();

		if (bRoverLoaded) {
			rover.drawFaces();
			if (!bTerrainSelected) drawAxis(rover.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}
	
	ofNoFill();
	ofSetColor(ofColor::white);

    if (intPtsToggle == true){
        for(int i = 0; i <6; i++)
            ofDrawSphere(boxPts[i].x(),boxPts[i].y(),boxPts[i].z(),.05);
    }
    
    //keyLight.draw();
    
    lander.drawFaces();
    exhaust.draw();
	ofPopMatrix();
	cam.end();
    theCam->end();
    
    string str;
    str += "Frame Rate: " + std::to_string(ofGetFrameRate());
    ofSetColor(ofColor::white);
    ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);
    
    string altistr;
    altistr += "Altitude: " + std::to_string(altitude);
    ofSetColor(ofColor::white);
    ofDrawBitmapString(altistr, 15, 15);
}

// 

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
        case '1':
            theCam = &cam;
            break;
        case '2':
            theCam = &trackingCam;
            break;
        case '3':
            theCam = &sideCam;
            break;
        case '4':
            theCam = &downCam;
            break;
        case '5':
            theCam = &cam;
            theCam->setPosition(glm::vec3(lander.getPosition().x + 10, lander.getPosition().y + 2, lander.getPosition().z + 10));
            theCam->lookAt(glm::vec3(lander.getPosition().x, lander.getPosition().y + 2, lander.getPosition().z));
            break;
        case '6':
            direc = lander.getPosition() - theCam->getPosition();
            theCam->setPosition(theCam->getPosition() + (direc.getNormalized() * -.2));
            break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
    case 'i':
        {
            // used for testing collisions
             ofVec3f vel = prover.particles[0].velocity;
             resForce->apply(-ofGetFrameRate()*vel);
            
            break;
        }
    case 'p':
            intPtsToggle = !intPtsToggle;
        break;
            
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
        theme.stop();
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
        case OF_KEY_UP:

            // Using alt key to determine whether we change z or x
            if (!bAltKeyDown){
                prover.reset();
                thrust->set(ofVec3f(0,SPD,0));
                exhaust.sys->reset();
	    		exhaust.start();
                ufo.play();
            }
            else{
                prover.reset();
                thrust->set(ofVec3f(0,0,SPD));
                
            }
            break;
        case OF_KEY_DOWN:
            if(!bAltKeyDown){
                prover.reset();
                thrust->set(ofVec3f(0,-SPD,0));
            }
            
            else{
                prover.reset();
                thrust->set(ofVec3f(0,0,-SPD));
            }
            
            break;
        case OF_KEY_LEFT:
            prover.reset();
            thrust->set(ofVec3f(-SPD,0,0));
            break;
        case OF_KEY_RIGHT:
            prover.reset();
            thrust->set(ofVec3f(SPD,0,0));
            break;
        default:
            break;
    }
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
    case OF_KEY_UP:
            exhaust.sys->reset();
            ufo.stop();
        exhaust.stop();
    case OF_KEY_RIGHT:
    case OF_KEY_LEFT:
    case OF_KEY_DOWN:
        prover.reset();
        thrust->set(ofVec3f(0,0,0));
        break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    
}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void ofApp::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

bool ofApp::doPointSelection() {

}

// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);

	if (rover.loadModel(dragInfo.files[0])) {
		rover.setScaleNormalization(false);
		rover.setScale(.005, .005, .005);
		rover.setPosition(point.x, point.y, point.z);
		bRoverLoaded = true;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	glm::vec3 mouse(mouseX, mouseY, 0);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

// Linhson Bui
// Casts a ray downwards from the ship until it collides with the terrain
// Calculates the distance and displays that on screen
//
void ofApp::altRayDistance(){
    
    // Create a ray where the point is the position of the spaceship
    // and the direction is striaght down
    ofVec3f rayPoint = prover.particles[0].position;
    ofVec3f rayDir = ofVec3f(0,-1,0);
    Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
                    Vector3(rayDir.x, rayDir.y, rayDir.z));
    
    TreeNode selectionNode;
    octree.intersect(ray, octree.root, selectionNode);
    
    
    // If the oct intersection returns a Node with a point
    // set the altitude to the difference of the Ship position and that returned node point
    if (selectionNode.points.size() > 0) {
        altPoint = mars.getMesh(0).getVertex(selectionNode.points[0]);

        ofVec3f resRay = rayPoint - altPoint;
        altitude = resRay.y;
    }
}

// Linhson Bui
// Detects collision using the center of each face of the bound box of the ship
//
void ofApp::collisionDect(){

    if(colDetected == false){
        
        // gets the center of each box face and puts thm
        // into an array
        boxPts[0] = shipBox.max() + Vector3(-shipW/2,0,-shipL/2);
        boxPts[1] = shipBox.min() + Vector3(0,shipH/2,shipL/2);
        boxPts[2] = shipBox.min() + Vector3(shipW/2,shipH/2,0);
        boxPts[3] = shipBox.max() + Vector3(0,-shipH/2,-shipL/2);
        boxPts[4] = shipBox.max() + Vector3(-shipW/2, -shipH/2,0);
        boxPts[5] = shipBox.min() + Vector3(shipW/2,0,shipL/2);
        
        // return of the ship is moving upwards
        // we have no ceilings in this terrain
        ofVec3f vel = prover.particles[0].velocity;
        if (vel.y > 0) return;
        
        int i = 0;
        
        TreeNode selectionNode;
        
        // we traverse through that array
        // seeing if they intersect with the terrain octree
        // once 1 is found set colDetected to true
        while( i < 8 && colDetected == false){
            
            octree.intersect(boxPts[i], octree.root, selectionNode);
            if(selectionNode.points.size() > 0){
               // cout << "Collision Detected" << endl;
                colDetected = true;
            }
            else i++;
        }
    }
}

// Linhson Bui
// Applies the resolution force to the ship if there is a collision
//
void ofApp::resCollision(){
    
    // stops checking for collision of the ship stops moving in the y direction
    ofVec3f vel = prover.particles[0].velocity;
    if (vel.y == 0){
        return;
    }
    
    // applying the formula from the lecture
    ofVec3f n = vel;
    n.normalize();
    float dot = (-vel.x * n.x) + (-vel.y * n.y) + (-vel.z * n.z);
    ofVec3f resVec = 1.2 * dot * n;
    
    resForce->apply(ofGetFrameRate()*resVec);
    //cout << "ResForce applied" << endl;
    colDetected = false;
    
}
