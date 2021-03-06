/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss




struct config{
       int L1blocksize;
       int L1setsize;
       int L1size;
       int L2blocksize;
       int L2setsize;
       int L2size;
    };

// you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {
    public:
    unsigned long cachesize;
    unsigned long blocksize;
    long maxidentifier;
    int setsize;
    int **array;
    int * size;
    
    cache(){}
    
    cache(int blocksize ,int setsize ,int size){
        this->blocksize = blocksize;
        this->cachesize = size*(long)pow(2,10);
        this->setsize = setsize;
        maxidentifier = (cachesize)/(blocksize*setsize);
        cout<<maxidentifier<<endl;
        cout<<setsize<<endl;
        array = new int *[maxidentifier];
        this->size = new int [maxidentifier];
        for(int i=0; i<maxidentifier; i++){
            *(array+i) = new int[setsize];
            *(this->size+i) = 0;
        }
    }

    bool read(unsigned long address){
        int tag = (address/blocksize)/maxidentifier;
        int index = (address/blocksize)%(maxidentifier);
        //cout<<index<<endl;
        //cout<<tag<<endl;
        for(int i=0;i<*(size+index);i++){
            if((*(*(array+index)+i) == tag)){
                updateLRU(index,i);
                return true;
            }
        }
        discardLRU(index);
        fetch(index,address);
        return false;        
    }

    int write(unsigned long address){
        int tag = (address/blocksize)/maxidentifier;
        int index = (address/blocksize)%(maxidentifier);
        for(int i=0;i<setsize;i++){
            if(*(*(array+index)+i) == tag){
                updateLRU(index,i);
                return true;
            }
        }
        return false; 
    }
    void discardLRU(int index){
        if( *(size+index) == setsize){
            for(int i=0;i<*(size+index)-1;i++)
                *(*(array+index)+i) = *(*(array+index)+i+1);
            *(size+index)-=1;
        }
    }
    void fetch(int index, unsigned long address){
        *(*(array+index)+*(size+index)) = (address/blocksize)/maxidentifier;
        //cout<<"array "<<index<<" "<<*(size+index)<<" =\t"<<*(*(array+index)+*(size+index));
        *(size+index)+=1;
        //cout<<" "<<*(size+index)<<endl;
    }

    void updateLRU(int index,int i){
        int latest = *(*(array+index)+i);
        for(i;i<*(size+index)-1;i++){
            *(*(array+index)+i) = *(*(array+index)+i+1);
        }

        *(*(array+index)+*(size+index)-1) = latest;
    }

    void printStatus(int index){
        for(int i=0;i<*(size+index);i++){
            cout<<*(*(array+index)+i)<<" ";
        }
        cout<<endl;
    }
};
       

int main(int argc, char* argv[]){


    
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]); //filename
    while(!cache_params.eof())  // read config file
    {
      cache_params>>dummyLine;
      cache_params>>cacheconfig.L1blocksize;
      cache_params>>cacheconfig.L1setsize;              
      cache_params>>cacheconfig.L1size;
      cache_params>>dummyLine;              
      cache_params>>cacheconfig.L2blocksize;           
      cache_params>>cacheconfig.L2setsize;        
      cache_params>>cacheconfig.L2size;
    }
    
  
    cache L1(cacheconfig.L1blocksize,cacheconfig.L1setsize,cacheconfig.L1size);
    cache L2(cacheconfig.L2blocksize,cacheconfig.L2setsize,cacheconfig.L2size);
    
    int L1AcceState =0;
    int L2AcceState =0;
   
   
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    
    string line;
    string accesstype;  
    string xaddr;       
    unsigned int addr;          
    bitset<32> accessaddr; 
    
    if (traces.is_open()&&tracesout.is_open()){    
        while (getline (traces,line)){  
            
            istringstream iss(line); 
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);
           
           
              if (accesstype.compare("R")==0)
              
             {                 
                    if(L1.read(accessaddr.to_ulong())){
                        L1AcceState = 1;
                        L2AcceState = 0;      
                    }
                    else{
                        L1AcceState = 2;
                        if(L2.read(accessaddr.to_ulong()))
                            L2AcceState = 1;
                        else L2AcceState = 2;
                    }
                 }
             else 
             {    
                  if(L1.write(accessaddr.to_ulong())){
                        L1AcceState = 3;
                        if(L2.write(accessaddr.to_ulong()))
                            L2AcceState = 3;
                        else L2AcceState = 4;
                    }
                    else{
                        L1AcceState = 4;
                        if(L2.write(accessaddr.to_ulong()))
                            L2AcceState = 3;
                        else L2AcceState = 4;
                    }
            }
            tracesout<< L1AcceState << " " << L2AcceState << endl;  
        }
        traces.close();
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";


    
    
  

   
    return 0;
}
