#include<iostream>
#include<vector>

using namespace std;
int main(){
    vector<uint8_t>v;
    // cout<<v[0]<<endl;
    size_t sz = v.size();
    v.resize(sizeof(float));
    float f=1.1f;
    memcpy(v.data()+sz,&f,sizeof(float));
    // v.push_back(f);
    
    f= 2.2f;
    float val;
    // cout<<int64_t(&f)<<endl;
    for(auto i: v)
        cout<<i<<" ";
    cout<<v.size()<<endl;
    memcpy(&val,v.data()+1,sizeof(float));
    cout<<val<<endl;

    cout<<sizeof(float)<<endl;
    return 0;
}