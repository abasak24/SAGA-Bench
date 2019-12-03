#include<iostream>
#include<fstream> 
#include<string>
#include<vector>
#include<cassert>
#include<cmath>

using namespace std;

void findStats(std::vector<float>& vec, float& min, float& max, float& average){
    assert (!vec.empty());
    min = vec[0]; max = vec[0]; average = 0; float sum = 0;
    vector<float>::iterator it;
    for(it = vec.begin(); it!=vec.end(); it++){
        sum += *it;
        if(*it > max) max = *it;
        if(*it < min) min = *it;
    }

    average = sum/vec.size();
}

int main(){
    ifstream in1("./PRDynAdListP.csv");
    ifstream in2("./PRStatAdListP.csv");

    string line1;
    string line2;

    vector<float> per_error;    

    while(!in1.eof()){
        getline(in1, line1); 
        //cout << "Line1 " << line1 << endl;
        getline(in2, line2);
        //cout << "Line2 " << line2 << endl;
        
        if(line1 != "" && line2 != ""){
            float val_dyn = stof(line1);
            float val_stat = stof(line2);
            float error;
            if(val_stat != 0){
                error = ((val_dyn - val_stat)/val_stat) * 100;
                //cout << error << endl;
                error = fabs(error); 
                //cout << error << endl;
                per_error.push_back(error);
            }else{
                if(val_dyn == val_stat) per_error.push_back(0);
                else{
                    error = ((val_dyn - val_stat)/(1.0e-10)) * 100;
                    per_error.push_back(fabs(error));
                }
            }            
        }        
    }

    /*while(!in1.eof()){
        getline(in1, line1); 

        if(line1 != "" ){
            float val_dyn = stof(line1);
            float val_stat = 4847570;

            float error = fabs(((val_dyn - val_stat)/(val_stat)) * 100);

            per_error.push_back(error);            
        }
    }*/

    float min;
    float max; 
    float average;

    findStats(per_error, min, max, average);

    cout << "Minimum error " << min << endl;
    cout << "Maximum error " << max << endl;
    cout << "Average error " << average << endl;
}