#include"ExEmbedEvent.h"

ExEmbedEvent::ExEmbedEvent(){
  ExEventV1();
  PrimPartList();

  //other variables
}

void ExEmbedEvent::Clear(Option_t* option){
  Reset();
}

ExEmbedEvent::~ExEmbedEvent(){
  Reset();
}

void ExEmbedEvent::Reset(){
  ExEventV1::Reset();
  PrimPartList::Reset();
}
