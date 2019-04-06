
/**
 *
 * toqutree (pa3)
 * significant modification of a quadtree .
 * toqutree.cpp
 * This file will be used for grading.
 *
 */

#include "toqutree.h"
#include "stats.h"


toqutree::Node::Node(pair<int,int> ctr, int dim, HSLAPixel a)
	:center(ctr),dimension(dim),avg(a),NW(NULL),NE(NULL),SE(NULL),SW(NULL)
	{}

toqutree::~toqutree(){
	clear(root);
}

toqutree::toqutree(const toqutree & other) {
	root = copy(other.root);
}


toqutree & toqutree::operator=(const toqutree & rhs){
	if (this != &rhs) {
		clear(root);
		root = copy(rhs.root);
	}
	return *this;
}



toqutree::toqutree(PNG & imIn, int k){ 
	cout<<"in toqutree constructor"<<endl;
/* This constructor grabs the 2^k x 2^k sub-image centered */
/* in imIn and uses it to build a quadtree. It may assume  */
/* that imIn is large enough to contain an image of that size. */

	//determine the center of imIn
	//POTENTIAL BUGS
	unsigned int inwidth = imIn.width();
	unsigned int inheight = imIn.height();

	unsigned int centx = inwidth/2;
	unsigned int centy = inheight/2;

	//Create new centered PNG
	PNG * data = new PNG(pow(2,k),pow(2,k));
	unsigned int xoffset = (inwidth - (unsigned int)pow(2,k))/2; 
	unsigned int yoffset = (inheight - (unsigned int)pow(2,k))/2;

	//for populating the new data PNG.
	for (unsigned int i = xoffset; i < (xoffset + (unsigned int)pow(2,k)); i++){
		for(unsigned int j = yoffset; j < (yoffset + (unsigned int)pow(2,k)); j++){
			//cout<<"in toqutree loop"<<endl;
			//int subi = int(i) - int(inwidth);
			//cout<<subi<<endl;
			//cout<<"i = "<<i<<endl;
			//cout<<"centx = "<<centx<<endl;
			//cout<<"offset = "<<xoffset<<endl;
			*data->getPixel((i-xoffset),(j-yoffset)) = *imIn.getPixel(i,j);
		}
	}

	//Build tree based off of new PNG
	root = buildTree(data,k);
}



int toqutree::size() {
	if (root == NULL){
		return 0;
	}
	return sizeHelp(root);
}

int toqutree::sizeHelp(toqutree::Node* n){
	if (n==NULL){
		return 0;
	}
	return 1 + sizeHelp(n->NW) + sizeHelp(n->NE) + sizeHelp(n->SE) + sizeHelp(n->SW);
}



toqutree::Node * toqutree::buildTree(PNG * im, int k) {

/* your code here */
//cout<<"in build tree"<<endl;
// Note that you will want to practice careful memory use
// In this function. We pass the dynamically allocated image
// via pointer so that it may be released after it is used .
// similarly, at each level of the tree you will want to 
// declare a dynamically allocated stats object, and free it
// once you've used it to choose a split point, and calculate
// an average.

	//create new dynamic stats object
	
	stats* temp = new stats(*im);

	//Find average for current PNG
	pair<int,int> topLeft(0,0);
	pair<int,int> bottomRight(int(im->width() - 1),int(im->height() - 1));
	//cout<<"coordinates = "<<int(im->width() -1)<<" "<<int(im->height() - 1)<<endl;
	HSLAPixel avg = temp->getAvg(topLeft,bottomRight);
	//cout<<"first avg"<<avg.h<<endl;
	//cout<<"first avg"<<avg.s<<endl;
	//cout<<"first avg"<<avg.l<<endl;
	//cout<<"first avg"<<avg.a<<endl;


	//base case
	if(im == NULL){
		return NULL;
	}

	

	//Base case for k=0
	if (k==0){
		pair<int,int> s(0,0);
		Node* newNodeptr = new Node(s,k,avg);

		newNodeptr->NW = NULL;
		newNodeptr->NE = NULL;
		newNodeptr->SE = NULL;
		newNodeptr->SW = NULL;

		delete(temp);

		return newNodeptr;
	}

	//Have base case for if the node now only has 4 pixels in it
	if (k==1){
		pair<int,int> s(0,0);
		Node* newNodeptr = new Node(s,k,avg);

		PNG * childNW = new PNG(1, 1);
		*childNW->getPixel(0,0) = *im->getPixel(1,1);
		PNG * childNE = new PNG(1, 1);
		*childNW->getPixel(0,0) = *im->getPixel(0,1);
		PNG * childSE = new PNG(1, 1);
		*childNW->getPixel(0,0) = *im->getPixel(0,0);
		PNG * childSW = new PNG(1, 1);
		*childNW->getPixel(0,0) = *im->getPixel(1,0);

		newNodeptr->NW = buildTree(childNW, k-1);
		newNodeptr->NE = buildTree(childNE, k-1);
		newNodeptr->SE = buildTree(childSE, k-1);
		newNodeptr->SW = buildTree(childSW, k-1);

		//free memeory
		delete (temp);
		delete (childNW);
		delete (childNE);
		delete (childSE);
		delete (childSW);

		return newNodeptr;
	}

	//Define dimensions of current PNG
	int fullwidth = int(pow(2,k));
	int start = int(pow(2,k-2));
	int subwidth = int(pow(2,k-1));
	int end = start + subwidth;


	//find entropy fo the start point
	pair<int,int> firstul(start,start);
	int firstlrx = (start + subwidth -1) % fullwidth;
	int firstlry = (start + subwidth -1) % fullwidth;
	pair<int,int> firstlr(firstlrx,firstlry);
	double minent = temp->entropy(firstul,firstlr);

	//initial split point
	pair<int,int> splitpoint = firstul;

	//for testing
	if(firstlrx < 0 || firstlry<0){
		cout<<"First NEGATIVE"<<endl;
	}

	//Iterate through possible split points looking the one that 
	//minimizes the entropy. If new min is found, change minimum entropy
	//and split point field
	for (int i = start; i < end; i++){
		for (int j = start; j < end; j++){
			pair<int,int> ul(i,j);
			int lrx = (i + subwidth - 1) % fullwidth;
			int lry = (j + subwidth - 1) % fullwidth;
			pair<int,int> lr(lrx,lry);

			//for testing
			if(lrx < 0 || lry<0){
				cout<<i<<"	"<<j<<"	NEGATIVE"<<endl;
			} 

			//if the split gives a lower entropy then
			// save it as the new split point and the 
			//new minimum entropy
			double tempent = temp->entropy(ul,lr);
			if (tempent < minent){
				minent = tempent;
				splitpoint = ul;
			}
		}
	}


	//create new node for current PNG in the tree
	Node* newNodeptr = new Node(splitpoint,k,avg);

	//We've now determined the optimal splitting point for a minimum entropy.
	//How to split up the png?
	//maybe create PNG's and then free them at the end of the function
	PNG * childNW = new PNG((unsigned int)subwidth, (unsigned int)subwidth );
	for (int i = splitpoint.first +subwidth; i < (splitpoint.first + 2*subwidth - 1) ; i++){
		for(int j = splitpoint.second + subwidth ; j < (splitpoint.second + 2*subwidth - 1) ; j++){
			//cout<<"in child NW"<<endl;
			unsigned int childi = (unsigned int)(i - splitpoint.first - subwidth);
			unsigned int childj = (unsigned int)(j - splitpoint.second - subwidth);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			//cout<<"childi = "<<childi<<endl;
			//cout<<"childj = "<<childj<<endl;
			//cout<<"xcoord = "<<xcoord<<endl;
			//cout<<"ycoord = "<<ycoord<<endl;

			*childNW->getPixel(childi, childj) = *im->getPixel(xcoord,ycoord);
		}
	}


	PNG * childNE = new PNG((unsigned int)subwidth, (unsigned int)subwidth);
	for (int i = splitpoint.first ; i < (splitpoint.first + subwidth - 1) ; i++){
		for(int j = splitpoint.second + subwidth ; j < (splitpoint.second + 2*subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first);
			unsigned int childj = (unsigned int)(j - splitpoint.second - subwidth);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*childNE->getPixel(childi, childj) = *im->getPixel(xcoord,ycoord);
		}
	}


	PNG * childSE = new PNG((unsigned int)subwidth, (unsigned int)subwidth);
	for (int i = splitpoint.first ; i < (splitpoint.first + subwidth - 1) ; i++){
		for(int j = splitpoint.second ; j < (splitpoint.second + subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first);
			unsigned int childj = (unsigned int)(j - splitpoint.second);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*childSE->getPixel(childi, childj) = *im->getPixel(xcoord,ycoord);
		}
	}

	PNG * childSW = new PNG((unsigned int)subwidth, (unsigned int)subwidth);
	for (int i = splitpoint.first + subwidth ; i < (splitpoint.first + 2*subwidth - 1) ; i++){
		for(int j = splitpoint.second ; j < (splitpoint.second + subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first - subwidth);
			unsigned int childj = (unsigned int)(j - splitpoint.second);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*childSW->getPixel(childi, childj) = *im->getPixel(xcoord,ycoord);
		}
	}



	//Create children nodes recrusively using sub PNGs
	//determined from the min entropy splitting point
	newNodeptr->NW = buildTree(childNW, k-1);
	newNodeptr->NE = buildTree(childNE, k-1);
	newNodeptr->SE = buildTree(childSE, k-1);
	newNodeptr->SW = buildTree(childSW, k-1);

	//free memeory
	delete (temp);
	delete (childNW);
	delete (childNE);
	delete (childSE);
	delete (childSW);

	return newNodeptr;
}


PNG toqutree::render(){

// My algorithm for this problem included a helper function
// that was analogous to Find in a BST, but it navigated the 
// quadtree, instead.


	PNG rval = *renderHelp(root);
	cout<<size()<<endl;
	return rval;

}


PNG* toqutree::renderHelp(toqutree::Node * n){
	if (n->NW == NULL && n->SW ==NULL && n->NE == NULL && n->SE==NULL){
		unsigned int shape = (unsigned int)pow(2,n->dimension);
		PNG* val = new PNG(shape,shape);
		for (unsigned int i = 0; i< shape; i++){
			for(unsigned int j = 0; j<shape; j++){
				*val->getPixel(i,j) = n->avg;
				cout<<"pixel"<<n->avg.h<<endl;
			}
		}

		return val;
	}


	//MAYBE PROBLEM WITH MEMEORY MANAGEMETN??????

	//I want to fill in val with the 4 sub images obtained through a recursive call to 
	//renderHelp
	pair<int,int> splitpoint = n->center;
	int fullwidth = int(pow(2,n->dimension));
	int subwidth = int(pow(2,n->dimension -1));
	//if (n->dim == 1){
	//	int start = 0;
	//} else if (n->dim != 1){
	//	int start = int(pow(2,n->dim - 2));
	//}
	

	unsigned int shape = (unsigned int)pow(2,n->dimension);
	PNG* val = new PNG(shape,shape);
	PNG* NWval = renderHelp(n->NW);
	PNG* NEval = renderHelp(n->NE);
	PNG* SEval = renderHelp(n->SE);
	PNG* SWval = renderHelp(n->SW);



	for (int i = splitpoint.first +subwidth; i < (splitpoint.first + 2*subwidth - 1) ; i++){
		for(int j = splitpoint.second + subwidth ; j < (splitpoint.second + 2*subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first - subwidth);
			unsigned int childj = (unsigned int)(j - splitpoint.second - subwidth);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*val->getPixel(xcoord, ycoord) = *NWval->getPixel(childi,childj);

		}
	}
	delete(NWval);

	for (int i = splitpoint.first ; i < (splitpoint.first + subwidth - 1) ; i++){
		for(int j = splitpoint.second + subwidth ; j < (splitpoint.second + 2*subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first);
			unsigned int childj = (unsigned int)(j - splitpoint.second - subwidth);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*val->getPixel(xcoord, ycoord) = *NEval->getPixel(childi,childj);
		}
	}
	delete(NEval);

	for (int i = splitpoint.first ; i < (splitpoint.first + subwidth - 1) ; i++){
		for(int j = splitpoint.second ; j < (splitpoint.second + subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first);
			unsigned int childj = (unsigned int)(j - splitpoint.second);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*val->getPixel(xcoord,ycoord) = *SEval->getPixel(childi, childj);
		}
	}
	delete(SEval);

	for (int i = splitpoint.first + subwidth ; i < (splitpoint.first + 2*subwidth - 1) ; i++){
		for(int j = splitpoint.second ; j < (splitpoint.second + subwidth - 1) ; j++){

			unsigned int childi = (unsigned int)(i - splitpoint.first - subwidth);
			unsigned int childj = (unsigned int)(j - splitpoint.second);
			unsigned int xcoord = (unsigned int)(i % fullwidth);
			unsigned int ycoord = (unsigned int)(j % fullwidth);

			*val->getPixel(xcoord, ycoord) = *SWval->getPixel(childi,childj);
		}
	}
	delete(SWval);
	//cout<<"size"<<size()<<endl;
	return val;
}

/* oops, i left the implementation of this one in the file! */
void toqutree::prune(double tol){

	prune(root,tol);

}

//Helper for prune
void toqutree::prune(Node* root, double tol){

	if (root == NULL){
		return;
	}
	//if criteria met prune the children
	double currc = root->avg.h;
	double NWc = root->NW->avg.h;
	double NEc = root->NE->avg.h;
	double SEc = root->SE->avg.h;
	double SWc = root->SW->avg.h;

	if (abs(currc - NWc) > tol && abs(currc - NEc) > tol && abs(currc - SEc) > tol && abs(currc - SWc) > tol ){
		delete (root->NW);
		delete (root->NE);
		delete (root->SE);
		delete (root->SW);
		return;
	} 

	//if not recuse on children
	prune (root->NW, tol);
	prune (root->NE, tol);
	prune (root->SE, tol);
	prune (root->SW, tol);

	return;
}


/* called by destructor and assignment operator*/
void toqutree::clear(Node * & curr){
	if (curr == NULL){
		return;
	}
	clearHelp(curr);
}

void toqutree::clearHelp(Node* & c){
	if (c == NULL){
		return;
	}
	clearHelp(c->NW);
	clearHelp(c->NE);
	clearHelp(c->SE);
	clearHelp(c->SW);
	delete (c);
	c=NULL;
}

/* done */
/* called by assignment operator and copy constructor */
//need to do preorder traversal
//THE NOTES said may need recursive helper function???? WHY
toqutree::Node * toqutree::copy(const Node * other) {
	Node* newTreePtr = NULL;

	if(other != NULL){
		newTreePtr = new Node(other->center,other->dimension,other->avg);
		newTreePtr->NW = copy(other->NW);
		newTreePtr->NE = copy(other->NE);
		newTreePtr->SE = copy(other->SE);
		newTreePtr->SW = copy(other->SW);
	}
	return newTreePtr;
}



