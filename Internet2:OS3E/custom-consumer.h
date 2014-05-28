//custom-consumer.h

//Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

#ifndef CUSTOM_CONS_H_
#define CUSTOM_CONS_H_

#include "ns3/ndn-app.h"
#include "ns3/ndn-pit.h"

namespace ns3 {

class CustomConsumer : public ndn::App
{
public:   
  static TypeId
  GetTypeId ();
  
  virtual void
  StartApplication ();
 
  virtual void
  StopApplication ();
 
  virtual void
  OnData (Ptr<const ndn::Data> contentObject);

private:
  ndn::Name prefix;
  double frequency;

  void 
  SendInterest ();

};

} // namespace ns3

#endif 

