//given the hit position (x,y) 
//on MpcEx and return the key
//of corresponding minipad

#ifndef __EXGEOMTOKEY_H__
#define __EXGEOMTOKEY_H__

#include <functional>
#include <map>
#include <math.h>

//double type key compare function
class own_double_less : public std::binary_function<double,double,bool>{
  public:
    own_double_less(double arg = 1e-7):_epsilon(arg){}
    bool operator()(const double &left,const double &right) const{
      return (fabs(left-right) > _epsilon) && (left < right);
    }
  private:
    double _epsilon;
};

class ExGeomToKey{
  public:
    virtual ~ExGeomToKey(){}
    static ExGeomToKey* instance();
    
    //get the key consider the position x ,y and layer
    //x,y are not in hough space !!! they are in cm 
    int get_key(int arm,int layer,double x,double y);
    //test check the number of the key in the map
    int get_num_keys();
    void print();
    
  private:
    ExGeomToKey();
    static ExGeomToKey* _instance;
    //order the key by the position of the x (or y)
    //and map x (y) to th key depend the type of the map
    typedef std::map<double,int,own_double_less> XYToKey;
    //group the layer minipad by the position of y (or x)
    typedef std::map<double,XYToKey,own_double_less> XYToXYMap;
    XYToXYMap _xy_to_xy_map[2][8];
    double _x_type_x_width;
    double _y_type_x_width;
    double _x_type_y_width;
    double _y_type_y_width;
};

#endif
