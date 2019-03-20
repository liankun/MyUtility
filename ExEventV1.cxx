#include "ExEventV1.h"
#include <MpcExEventHeader.h>

ExEventV1::ExEventV1(){
  ExEvent();
  _stack = -9999;
  for(int i=0;i<MpcExConstants::NARMS;i++){
    for(int j=0;j<MpcExConstants::NPACKETS_PER_ARM;j++){
      _statephase[i][j] = -9999;
      _parsttime[i][j] = -9999;
    }
  }
}

void ExEventV1::Clear(Option_t*){
  Reset();
}

ExEventV1::~ExEventV1(){
  Reset();
}

void ExEventV1::Reset(){
  ExEvent::Reset();
  _stack = -9999;
  for(int i=0;i<MpcExConstants::NARMS;i++){
    for(int j=0;j<MpcExConstants::NPACKETS_PER_ARM;j++){
      _statephase[i][j] = -9999;
      _parsttime[i][j] = -9999;
    }
  }
}

void ExEventV1::SetMpcExEventHeader(MpcExEventHeader* ex_header){
  if(!ex_header) return;
  _stack = ex_header->getStack();
  for(int i=0;i<MpcExConstants::NARMS;i++){
    for(int j=0;j<MpcExConstants::NPACKETS_PER_ARM;j++){
      _statephase[i][j] = ex_header->getStatephase(i*MpcExConstants::NPACKETS_PER_ARM + j) & 0xfff;;
      _parsttime[i][j] = (int) ex_header->getPARSTTime(i*MpcExConstants::NPACKETS_PER_ARM+j);
    }
  }
}
