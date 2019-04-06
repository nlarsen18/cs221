
#include "stats.h"

vector< vector< double >> sumHueX;
vector< vector< double >> sumHueY;
vector< vector< double >> sumSat;
vector< vector< double >> sumLum;

vector<vector<vector<int>>> hist;

stats::stats(PNG & im){
	unsigned int x = im.width();
	unsigned int y = im.height();
	//int ix = int x;
	//int iy = int y;

	//1==h
	//here we take the cos(h*PI/180), HX
	sumHueX.resize(x, vector<double>(y));
	sumHelp(im,1,0);

	//1==h
	//sin(h), HY
	sumHueY.resize(x, vector<double>(y));
	sumHelp(im,1,1);

	//2==s
	//normal pixel val, SAT
	sumSat.resize(x, vector<double>(y));
	sumHelp(im,2,3);

	//3==l
	//normal pixle val, LUM
	sumLum.resize(x, vector<double>(y));
	sumHelp(im,3,3);

	// in hist we use the normal h values from the pixels
	hist.resize(x, vector<vector<int>> (y, vector<int>(36)));


	for (unsigned int i = 0; i < x; i++){
		for (unsigned int j = 0; j < y; j++){
			HSLAPixel* currpix = im.getPixel(i,j);
			double currh = currpix->h;
			int index = floor(currh/10);

			//at origin
			if (i==0 && j==0){
				vector<int> temp = hist[i][j];
				temp[index] ++;
				hist[i][j] = temp;

			//on left edge
			} else if (i==0 && j!=0){
				vector<int> temp = hist[i][j-1];
				temp[index] ++;
				hist[i][j] = temp;

			//ontop edge
			} else if (i!=0 && j==0){
				vector<int> temp = hist[i-1][j];
				temp[index] ++;
				hist[i][j] = temp;

			// on no edge
			} else if (i!=0 && j!= 0){

				//account for sub-histograms
				for (int k = 0; k < 36; k++){
					hist[i][j][k] = hist[i-1][j][k] + hist[i][j-1][k] - hist[i-1][j-1][k];
				}
				//add current cell
				hist[i][j][index]++;
			}
		}
	}
}



void stats::sumHelp(PNG & im, int p,int c){
	unsigned int x = im.width();
	unsigned int y = im.height();

	for (unsigned int i = 0; i < x; i++){
		for (unsigned int j = 0; j < y; j++){

			HSLAPixel* currpix = im.getPixel(i,j);

			//HueX or HueY
			if (p==1){
				//Huex
				if(c==0){

					double curr = cos((currpix->h)*PI/180);
					//at origin
					if (i==0 && j==0){
						sumHueX[i][j] = curr;
			
					//on left edge
					} else if (i==0 && j!=0){
						sumHueX[i][j] = curr + sumHueX[i][j-1];

					//ontop edge
					} else if (i!=0 && j==0){
						sumHueX[i][j] = curr + sumHueX[i-1][j];

					// on no edge
					} else if (i!=0 && j!= 0){
						sumHueX[i][j] = curr + sumHueX[i-1][j] + sumHueX[i][j-1] - sumHueX[i-1][j-1];
					}

				//HueY
				} else if (c==1) {

					double curr = sin((currpix->h)*PI/180);
					//at origin
					if (i==0 && j==0){
						sumHueY[i][j] = curr;
			
					//on left edge
					} else if (i==0 && j!=0){
						sumHueY[i][j] = curr + sumHueY[i][j-1];

					//ontop edge
					} else if (i!=0 && j==0){
						sumHueY[i][j] = curr + sumHueY[i-1][j];

					// on no edge
					} else if (i!=0 && j!= 0){
						sumHueY[i][j] = curr + sumHueY[i-1][j] + sumHueY[i][j-1] - sumHueY[i-1][j-1];
					}
				}

			} else if (p==2){

				double curr = currpix->s;
				//at origin
				if (i==0 && j==0){
					sumSat[i][j] = curr;
			
				//on left edge
				} else if (i==0 && j!=0){
					sumSat[i][j] = curr + sumSat[i][j-1];

				//ontop edge
				} else if (i!=0 && j==0){
					sumSat[i][j] = curr + sumSat[i-1][j];

				// on no edge
				} else if (i!=0 && j!= 0){
					sumSat[i][j] = curr + sumSat[i-1][j] + sumSat[i][j-1] - sumSat[i-1][j-1];
				}

			} else if (p==3){

				double curr = currpix->l;
				//at origin
				if (i==0 && j==0){
					sumLum[i][j] = curr;
			
				//on left edge
				} else if (i==0 && j!=0){
					sumLum[i][j] = curr + sumLum[i][j-1];

				//ontop edge
				} else if (i!=0 && j==0){
					sumLum[i][j] = curr + sumLum[i-1][j];

				// on no edge
				} else if (i!=0 && j!= 0){
					sumLum[i][j] = curr + sumLum[i-1][j] + sumLum[i][j-1] - sumLum[i-1][j-1];
				}

			}
		}
	}
}


//Cases
//1. Normal cases when all coords of ul are less than lr
//2. ul.x > lr.x, ie the y values are normal
//3. ul.y > lr.y, ie the x values are normal
//4. All coords of ul are greater than lr

// we have access to the field hist. Therefore image width = hist.size()
// and image height = hist[0].size()
long stats::rectArea(pair<int,int> ul, pair<int,int> lr){
	double width = hist.size();
	double height = hist[0].size();

	//CASE ul==lr so 1x1
	//if (ul.first == lr.first && ul.second == lr.second){
	//	return 1;
	//}

	//1. Normal cases when all coords of ul are less than lr
	if (ul.first <= lr.first && ul.second <= lr.second){
		//I have a +1 here because the lr endpoint is inclusive, as shown online
		return (lr.first - ul.first + 1)*(lr.second - ul.second + 1);

	//2. ul.x > lr.x, ie the y values are normal
	} else if (ul.first > lr.first && ul.second <= lr.second){
		return (width - ul.first + lr.first + 1)*(lr.second - ul.second + 1);

	//3. ul.y > lr.y, ie the x values are normal
	} else if (ul.first <= lr.first && ul.second > lr.second){
		return (lr.first - ul.first + 1)*(height - ul.second + lr.first + 1);

	//4. All coords of ul are greater than lr
	} else if (ul.first > lr.first && ul.second > lr.second){
		return (width - ul.first + lr.first + 1)*(height - ul.second + lr.first + 1);
	}
	cout<<"no case"<<endl;
}

//////////////////////////////////////////////////////////////////////////////////////

HSLAPixel stats::getAvg(pair<int,int> ul, pair<int,int> lr){
	double hueX = helpGetAvg(ul, lr, sumHueX);
	double hueY = helpGetAvg(ul, lr, sumHueY);
	//cout<<"hueX"<<hueX<<endl;
	//cout<<"hueY"<<hueY<<endl;
	double hue = atan2(hueY, hueX)*180/PI;
	
	if (hue<0)
		hue = hue + 360;
	//This may need to be slightly different, potential bug !!!
	//while (hue < 0)
	//	hue += 360;
	//cout<<"ulx = "<<ul.first<<"uly = "<<ul.second<<endl;
	//cout<<"lrx = "<<lr.first<<"lry = "<<lr.second<<endl;
	double saturation = helpGetAvg(ul, lr, sumSat);
	double luminance = helpGetAvg(ul, lr, sumLum);
	//cout<<"h = "<<hue<<"s = "<<saturation<<endl;
	return HSLAPixel(hue, saturation, luminance, 1);
	
}


double stats::helpGetAvg(pair<int,int> ul, pair<int,int> lr, vector< vector< double>> channel){
	//first calculate the average channel valueover the 4 squares involved in 
	// finding the ul to lr average.
	//first = x, second = y
	//1. Normal cases when all coords of ul are less than lr
	double rval = 0; 
	//this initialization may be a source of bugs, check if theres erros in result!!

	//Case for 1x1
	//if (ul.first == lr.first && ul.second == lr.second){
	//	rval = channel[ul.first][ul.second];
	//}

	if (ul.first <= lr.first && ul.second <= lr.second){
		rval = helpAllNorm(ul, lr, channel);
		//cout<<"case 0"<<endl;

	//2. ul.x > lr.x, ie the y values are normal
	} else if (ul.first > lr.first && ul.second <= lr.second){
		rval = helpYNorm(ul, lr, channel);
		//cout<<"case 1"<<endl;
	//3. ul.y > lr.y, ie the x values are normal
	} else if (ul.first <= lr.first && ul.second > lr.second){
		rval = helpXNorm(ul, lr, channel);

		//cout<<"case 2"<<endl;
	//4. All coords of ul are greater than lr
	} else if (ul.first > lr.first && ul.second > lr.second){
		rval = helpNoNorm(ul, lr, channel);
		//cout<<"case 3"<<endl;
	}

	//Do we need to do antyhing before returning with rval?
	return rval;
}


double stats::helpAllNorm(pair<int,int> ul, pair<int,int> lr, vector< vector< double>> channel){ 
	double rval = 0;
	long area = rectArea(ul, lr);

	//snug into origin
	if(ul.first == 0 && ul.second == 0){
		//cout<<"section 0"<<endl;
		rval = channel[lr.first][lr.second] / area;

	//only left edge is against boundary
	} else if (ul.first == 0 && ul.second != 0){
		//cout<<"section 1"<<endl;
		rval = (channel[lr.first][lr.second] - channel[lr.first][ul.second-1]) / area;

	// only top edge is against boundary
	} else if (ul.first != 0 && ul.second == 0){
		//cout<<"section 2"<<endl;
		rval = (channel[lr.first][lr.second] - channel[ul.first - 1][lr.second]) / area;

	// no edge is against top or left boundary
	} else if (ul.first != 0 && ul.second != 0){
		//cout<<"section 3"<<endl;
		rval = (channel[lr.first][lr.second] - channel[ul.first - 1][lr.second] - channel[lr.first][ul.second - 1] + channel[ul.first - 1][ul.second - 1]) / area;
	}

	return rval;
}

double stats::helpYNorm(pair<int,int> ul, pair<int,int> lr, vector< vector< double>> channel){
	double rval = 0;
	long area = rectArea(ul, lr);
	double width = hist.size();
	//double height = hist[0].size();

	//Touching top edge
	if (ul.second == 0){
		rval = (channel[width - 1][lr.second] - channel[ul.first - 1][lr.second] + channel[lr.first][lr.second]) / area;

	//Not touching top edge
	} else if(ul.second != 0){
		rval = (channel[width - 1][lr.second] - channel[width - 1][ul.second - 1] - channel[ul.first - 1][lr.second] + channel[ul.first - 1][ul.second - 1] + channel[lr.first][lr.second] - channel[lr.first][ul.second - 1]) / area;
	}

	return rval;

}

double stats::helpXNorm(pair<int,int> ul, pair<int,int> lr, vector< vector< double>> channel){
	double rval  = 0;
	long area = rectArea(ul, lr);
	//double width = hist.size();
	double height = hist[0].size();

	//Touching left edge
	if (ul.first == 0){
		rval = (channel[lr.first][height - 1] - channel[lr.first][ul.second - 1] + channel[lr.first][lr.second]) / area;

	//Not touching left edge
	} else if (ul.first != 0 ){
		rval = (channel[lr.first][height - 1] - channel[lr.first][ul.second - 1] - channel[ul.first - 1][height - 1] + channel[ul.first - 1][ul.second - 1] + channel[lr.first][lr.second] - channel[ul.first - 1][lr.second]) / area;
	}

	return rval;
}

double stats::helpNoNorm(pair<int,int> ul, pair<int,int> lr, vector< vector< double>> channel){
	double rval = 0;
	long area = rectArea(ul,lr); 
	double width = hist.size();
	double height = hist[0].size();

	//Seems like there is only one case
	rval = (channel[width - 1][height - 1] - channel[width - 1][ul.second - 1] - channel[ul.first - 1][height - 1] + channel[ul.first - 1][ul.second - 1] + channel[width - 1][lr.second] + channel[lr.first][height - 1] - channel[ul.first - 1][lr.second] - channel[lr.first][ul.second - 1] + channel[lr.first][lr.second]) / area;

	return rval;
}


//////////////////////////////////////////////////////////////////////////////////

vector<int> stats::buildHist(pair<int,int> ul, pair<int,int> lr){
	vector<int> rval;
	rval.resize(36);
	//this initialization may be a source of bugs, check if theres erros in result!!

	//CASE where size is 1x1
	//if (ul.first == lr.first && ul.second == lr.second){
	//	cout<<"in Case"<<endl;
	//	rval = hist[ul.first][ul.second];
	//}

	//1. Normal cases when all coords of ul are less than lr
	if (ul.first <= lr.first && ul.second <= lr.second){
		rval = helpHistAllN(ul, lr);

	//2. ul.x > lr.x, ie the y values are normal
	} else if (ul.first > lr.first && ul.second <= lr.second){
		rval = helpHistYN(ul, lr);

	//3. ul.y > lr.y, ie the x values are normal
	} else if (ul.first <= lr.first && ul.second > lr.second){
		rval = helpHistXN(ul, lr);

	//4. All coords of ul are greater than lr
	} else if (ul.first > lr.first && ul.second > lr.second){
		rval = helpHistNoN(ul, lr);
	}

	//Do we need to do antyhing before returning with rval?
	return rval;
}

vector<int> stats::helpHistAllN(pair<int,int> ul, pair<int,int> lr){
	vector<int> rval;
	rval.resize(36);

	//CASE where size is 1x1
	if (ul.first == lr.first && ul.second == lr.second){
		cout<<"in Case"<<endl;
		rval = hist[ul.first][ul.second];
	}

	//snug into origin
	if(ul.first == 0 && ul.second == 0){
		rval = hist[lr.first][lr.second];

	//only left edge is against boundary
	} else if (ul.first == 0 && ul.second != 0){

		//test if its vertical or horizontal
		//cout<<"in left edge case"<<endl;
		//Vertical
		if (lr.second > ul.second){
			for (int k = 0; k<36; k++){
				rval[k] = hist[lr.first][lr.second][k] - hist[lr.first][ul.second-1][k];
			}
		//Horizontal
		} else if (lr.second == ul.second){
			//cout<<"in horizontal case"<<endl;
			for (int k = 0; k<36; k++){
				//cout<<"upper rect val "<<hist[lr.first][lr.second - 1][k]<<endl;
				//cout<<"full rect val "<<hist[lr.first][lr.second][k]<<endl;
				rval[k] = hist[lr.first][lr.second][k] - hist[lr.first][lr.second-1][k];
				//cout<<"horz worked"<<endl;

			}
		}	

	// only top edge is against boundary
	} else if (ul.first != 0 && ul.second == 0){
		for(int k = 0; k<36; k++){
			rval[k] = hist[lr.first][lr.second][k] - hist[ul.first - 1][lr.second][k];
		}

	// no edge is against top or left boundary
	} else if (ul.first != 0 && ul.second != 0){
		for(int k = 0; k<36; k++){
			rval[k] = hist[lr.first][lr.second][k] - hist[ul.first - 1][lr.second][k] - hist[lr.first][ul.second - 1][k] + hist[ul.first - 1][ul.second - 1][k];
		}
	}
	return rval;
}

vector<int> stats::helpHistYN(pair<int,int> ul, pair<int,int> lr){
	vector<int> rval;
	rval.resize(36);
	double width = hist.size();

	//Touching top edge
	if (ul.second == 0){
		for(int k = 0; k<36; k++){
			rval[k] = hist[width - 1][lr.second][k] - hist[ul.first - 1][lr.second][k] + hist[lr.first][lr.second][k];
		}
	//Not touching top edge
	} else if(ul.second != 0){
		for(int k = 0; k <36; k++){
			rval[k] = hist[width - 1][lr.second][k] - hist[width - 1][ul.second - 1][k] - hist[ul.first - 1][lr.second][k] + hist[ul.first - 1][ul.second - 1][k] + hist[lr.first][lr.second][k] - hist[lr.first][ul.second - 1][k];
		}
	}
	return rval;
}

vector<int> stats::helpHistXN(pair<int,int> ul, pair<int,int> lr){
	vector<int> rval;
	rval.resize(36);
	double height = hist[0].size();

	//Touching left edge
	if (ul.first == 0){
		for(int k = 0; k<36; k++){
			rval[k] = hist[lr.first][height - 1][k] - hist[lr.first][ul.second - 1][k] + hist[lr.first][lr.second][k];
		}

	//Not touching left edge
	} else if (ul.first != 0 ){
		for(int k = 0; k<36; k++){
			rval[k] = hist[lr.first][height - 1][k] - hist[lr.first][ul.second - 1][k] - hist[ul.first - 1][height - 1][k] + hist[ul.first - 1][ul.second - 1][k] + hist[lr.first][lr.second][k] - hist[ul.first - 1][lr.second][k];
		}
	}
	return rval;
}

vector<int> stats::helpHistNoN(pair<int,int> ul, pair<int,int> lr){
	vector<int> rval;
	rval.resize(36);
	double width = hist.size();
	double height = hist[0].size();

	//Seems like there is only one case
	for(int k = 0; k<36; k++){
		rval[k] = hist[width - 1][height - 1][k] - hist[width - 1][ul.second - 1][k] - hist[ul.first - 1][height - 1][k] + hist[ul.first - 1][ul.second - 1][k] + hist[width - 1][lr.second][k] + hist[lr.first][height - 1][k] - hist[ul.first - 1][lr.second][k] - hist[lr.first][ul.second - 1][k] + hist[lr.first][lr.second][k];
	}
	return rval;
}



// takes a distribution and returns entropy
// partially implemented so as to avoid rounding issues.
double stats::entropy(vector<int> & distn,int area){

    double entropy = 0.;

    for (int i = 0; i < 36; i++) {
        if (distn[i] > 0 ) 
            entropy += ((double) distn[i]/(double) area) 
                                    * log2((double) distn[i]/(double) area);
    }

    return  -1 * entropy;

}

double stats::entropy(pair<int,int> ul, pair<int,int> lr){

	//create distn using buildHist
	//create area using rectArea
	//cout<<"before buildHist"<<endl;
	vector<int> distn = buildHist(ul,lr);
	//cout<<"done buidHist"<<endl;
	long area = rectArea(ul,lr); 
	//cout<<"done rectArea"<<endl;

	return entropy(distn, area);

}
