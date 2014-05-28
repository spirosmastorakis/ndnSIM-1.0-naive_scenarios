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

#include "ns3/ndn-pit.h"
#include "ns3/ndn-fib.h"
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
  
  //FIB and PIT object
  //Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();
  Ptr<ndn::Pit> pit = GetNode ()->GetObject<ndn::Pit> ();

  //Set maximum pit entry lifetime
  pit->SetMaxPitEntryLifetime (Seconds(1.0));
  
  //Add entry to FIB
  //Ptr<ndn::fib::Entry> fibEntry = fib->Add (prefix, m_face, 0);
  CustomConsumer::SendInterest(/*pit, fibEntry*/);  

  //Simulator::Schedule (Seconds (1.0), CustomConsumer::SendInterest(pit, fibEntry), this);
}
//Application is stopped
void
CustomConsumer::StopApplication ()
{
  //Cleanup ndn::App
  ndn::App::StopApplication (); 
}

void
CustomConsumer::SendInterest (/*Ptr<ndn::Pit> pit, Ptr<ndn::fib::Entry> fibEntry*/)
{  
  //Create and configure ndn::Interest
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));
  
  NS_LOG_DEBUG ("Sending Interest packet for " << prefix);
 /*
  //PIT Lookup
  Ptr<ndn::pit::Entry> pitEntry = pit->Lookup(*interest);

  if (pitEntry == 0) {
        //Add PIT entry
        ndn::pit::Entry (*pit, interest, fibEntry);
        ndn::pit::IncomingFace (m_face);
  }
  else {
        //Update pitEntry life time
        (*pitEntry).UpdateLifetime (Seconds (1.0));
  	//Check if the current incoming face has been added to the list of incoming faces
        const std::set<ndn::pit::IncomingFace>& faces = (*pitEntry).GetIncoming ();
        if (faces.find (m_face) == faces.end ())
		//If it has not been added then add it!
		(*pitEntry).AddIncoming (m_face);
  }
*/
  //Call trace
  m_transmittedInterests (interest, this, m_face);

  m_face->ReceiveInterest (interest); 
}

//Data arrives
void
CustomConsumer::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_DEBUG ("Receiving Data packet for " << contentObject->GetName ());

  //TODO: Content store  

  //std::cout << "DATA received for name " << contentObject->GetName () << std::endl;
}

} //ns3
