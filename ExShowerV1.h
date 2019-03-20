#ifndef __ExShowerV1__
#define __ExShowerV1__

#include "ExEvent.h"

//Shower V1, add Mpc pulse qualtiy
//add Mpc Tower Pulse fitting info
class ExShowerV1 : public ExShower{
  private:
    float _quality[5][5];
  public:
    ExShowerV1();
    virtual ~ExShowerV1();
    virtual void ResetExShower();
    virtual void Clear(Option_t *option="");
    virtual void InitShower();
   
    void SetMpcPulseQuality(float quality[5][5]);
    float GetMpcPulseQuality(int l, int k) const {if(l>=0&&l<5&&k>=0&&k<5) return _quality[l][k]; return -9999;}
  
  ClassDef(ExShowerV1,1)
};


#endif
