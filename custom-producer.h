//custom-producer.h

//Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

#ifndef CUSTOM_APP_H_
#define CUSTOM_APP_H_

#include "ns3/ndn-name.h"
#include "ns3/ndn-app.h"
#include "ns3/ndn-pit.h"

namespace ns3 {

class CustomApp : public ndn::App
{
public:  
  //Register NS-3 type "CustomApp"
  static TypeId
  GetTypeId ();
  
  //Start of the application
  virtual void
  StartApplication ();

  //Application is stopped
  virtual void
  StopApplication ();

  //Interest arrives
  virtual void
  OnInterest (Ptr<const ndn::Interest> interest);

private:
  ndn::Name prefix;
  uint32_t Payload;
  uint32_t Num_nodes;
  Time sim_time;
};

} // namespace ns3

#endif // CUSTOM_APP_H_