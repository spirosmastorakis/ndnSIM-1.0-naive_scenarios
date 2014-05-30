//custom-consumer.h

//Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

#ifndef CUSTOM_CONS_H_
#define CUSTOM_CONS_H_

#include <vector>
#include "ns3/random-variable.h"
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
  RandomVariable *random_var; 
  EventId sendEvent; 
  EventId RTOevent;
  uint32_t seq;
  Time RTO;
  int trasmitted_packets; 

  struct SeqContainer {
 
        std::vector<Time> stamps;
        std::vector<uint32_t> seqs;
};	

  SeqContainer Retr_container;

  std::vector<EventId> RTO_Events; 
 
  void
  SchedulePacket ();

  void 
  SendInterest ();
  
  void
  RTO_Occured (uint32_t this_seq);
  
  void
  OutOfOrder (uint32_t out_of_order_seq);

};

} // namespace ns3

#endif 

