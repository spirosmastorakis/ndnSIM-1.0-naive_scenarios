//custom-consumer.cc

#include <set>
#include "custom-consumer.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/ndn-name.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("CustomConsumer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CustomConsumer);

//NS-3 type
TypeId
CustomConsumer::GetTypeId ()
{
  static TypeId tid = TypeId ("CustomConsumer")
    .SetParent<ndn::App> ()
    .AddConstructor<CustomConsumer> ()
    .AddAttribute ("Prefix", "data prefix", StringValue ("/"),MakeNameAccessor (&CustomConsumer::prefix),ndn::MakeNameChecker ())
    .AddAttribute ("Frequency", "Frequency of interest packets", StringValue ("1.0"),MakeDoubleAccessor (&CustomConsumer::frequency), MakeDoubleChecker<double> ()); 
  return tid;
}

//Start of the application
void
CustomConsumer::StartApplication ()
{
  //ndn::App
  ndn::App::StartApplication (); 
  //TODO: Implement interest sending policy
  Simulator::Schedule (Seconds (1.0), &CustomConsumer::SendInterest, this);
  Simulator::Schedule (Seconds (8.0), &CustomConsumer::SendInterest, this);
}
//Application is stopped
void
CustomConsumer::StopApplication ()
{
  //Cleanup ndn::App
  ndn::App::StopApplication (); 
}

void
CustomConsumer::SendInterest ()
{  
  //Create and configure ndn::Interest
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  srand(time(NULL));
  int num = rand();
  ndn::Name prefix_without_seq = prefix;
  //Random sequence number
  //TODO: Implement retransmission policy
  prefix.appendSeqNum (num);  
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));
  
  NS_LOG_DEBUG ("Sending Interest packet for " << prefix);

  //Call trace
  m_transmittedInterests (interest, this, m_face);

  m_face->ReceiveInterest (interest); 
}


//Data arrives
void
CustomConsumer::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << contentObject->GetName ());
 
}


