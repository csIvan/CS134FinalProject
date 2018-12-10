//  Kevin M. Smith - Basic Octree Class - CS134/235 4/18/18
//
// student: Ivan Hernandez

#include "Octree.h"
 

// draw Octree (recursively)
//
// added some functionality for it to handle colors for each box it draws
void Octree::draw(TreeNode & node, int numLevels, int level, int colorInt) {
	//changes the color of the box according to the colorInt passed in
	switch (colorInt) {
		case 0 : ofSetColor(ofColor::red);
			break;
		case 1 : ofSetColor(ofColor::blue);
			break;
		case 2: ofSetColor(ofColor::yellow);
			break;
		case 3: ofSetColor(ofColor::purple);
			break;
		case 4: ofSetColor(ofColor::green);
			break;
		default:
			break;
	}
	if (level >= numLevels) return;
	colorInt = (colorInt + 1) % 5;
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level, colorInt);
	}
}

// draw only leaf Nodes
//
void Octree::drawLeafNodes(TreeNode & node) {
	//if the node has no children, then draw it, this is the base case
	if (node.children.size() == 0) {
		drawBox(node.box);
	}
	else {	//otherwise since the node has children recursive call
		for (int i = 0; i < node.children.size(); i++) {
			drawLeafNodes(node.children[i]);
		}
	}
}


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
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
Box Octree::meshBounds(const ofMesh & mesh) {
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
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}



//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
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

// create the tree by initializing the root variables and subdivide to all the levels
void Octree::create(const ofMesh & geo, int numLevels) {
	// initialize octree structure
	//
	mesh = geo;
	root.box = meshBounds(mesh);
	//get the number of points from the mesh to set to the root.points
	vector<ofIndexType> meshPoints = mesh.getIndices();
	vector<int> temp(meshPoints.size());
	for (int i = 0; i < meshPoints.size(); i++) {
		temp[i] = meshPoints[i];
	}
	root.points = temp;
	//subdivide the numLevels passed in
	subdivide(mesh, root, numLevels, 0);

}

void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {

	//create a vector for the children boxes of the node passed in
	vector<Box> subBoxes(8);
	//call subDivideBox8() to divide the node's box into 8 boxes and set them to subBoxes
	subDivideBox8(node.box, subBoxes);

	for (int i = 0; i < subBoxes.size(); i++) {
		//create a child to serve as one of the subBoxes that were divided
		TreeNode newChild;
		newChild.box = subBoxes[i];
		level++;

		//get the amount of points that the child box has in the mesh
		int pointCount = getMeshPointsInBox(mesh, node.points, newChild.box, newChild.points);

		//if the child box has any points in the mesh
		if (pointCount > 0) {
			//if the child has at least 8 points, one for each box, then push the newChild
			if (pointCount <= subBoxes.size()) {
				node.children.push_back(newChild);
			}
			else {	//otherwise recursive call until it reaches base case and add the newChild
				subdivide(mesh, newChild, numLevels, level);
				node.children.push_back(newChild);
			}
		}
	}
}

bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
	//check if the ray intersects the node's box parameters
	if (node.box.intersect(ray, -7000, 7000)) {

		//if it is a nodeleaf, then set the nodeRtn and return true
		if (node.children.size() == 0) {
			nodeRtn = node;
			return true;
		}
		else {//otherwise recursive call until base case is reached
			for (int i = 0; i < node.children.size(); i++) {
				intersect(ray, node.children[i], nodeRtn);
			}
		}
	}
	return false;
}

bool Octree::intersect(const Vector3 &pt, const TreeNode & node, TreeNode & nodeRtn) {
    // checks to see if a point is intersects with the node
    Ray ray = Ray(pt,Vector3(0,0,0));
    
    if(node.box.intersect(ray, -7000, 7000)){
        
        //if it is a nodeleaf, then set the nodeRtn and return true
        if (node.children.size() == 0) {
            nodeRtn = node;
            return true;
        }
        else {//otherwise recursive call until base case is reached
            for (int i = 0; i < node.children.size(); i++) {
                intersect(ray, node.children[i], nodeRtn);
            }
        }
    }
    
    
    return false;
}


