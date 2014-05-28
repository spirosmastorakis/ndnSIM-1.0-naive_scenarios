//custom-producer.cc

//Statistics for incoming interests are kept
//Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

#include <boost/functional/hash.hpp>
#include <vector>
#include "custom-producer.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/ndn-name.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/core-module.h"
#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"

NS_LOG_COMPONENT_DEFINE ("CustomProducer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CustomProducer);

//Simple statistics array
int *stats, counter=0;
std::vector<std::string> names;

//NS-3 type
TypeId
CustomProducer::GetTypeId ()
{
  static TypeId tid = TypeId ("CustomProducer")
    .SetParent<ndn::App> ()
    .AddConstructor<CustomProducer> ()
    .AddAttribute ("Prefix", "Data Prefix", StringValue ("/"), ndn::MakeNameAccessor (&CustomProducer::prefix), ndn::MakeNameChecker ())
    .AddAttribute ("DataSize", "Payload Size", UintegerValue (1024), MakeUintegerAccessor (&CustomProducer::Payload), MakeUintegerChecker <uint32_t> ())
    .AddAttribute ("Node number", "Number of Topology Nodes", UintegerValue (1024), MakeUintegerAccessor (&CustomProducer::Num_nodes), MakeUintegerChecker <uint32_t> ())
    .AddAttribute ("Sim time", "Simulation time", TimeValue (Seconds(0)), MakeTimeAccessor (&CustomProducer::sim_time), MakeTimeChecker ());
  return tid;
}

//Start of the application
void
CustomProducer::StartApplication ()
{
  //ndn::App
  ndn::App::StartApplication ();

  //FIB object
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();
   
  //Memory allocation
  stats = (int*) calloc(Num_nodes, sizeof(int));
  names.clear();
  //Add entry to FIB
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (prefix, m_face, 0);
  //Make face green
  fibEntry->UpdateStatus (m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);
  //Print statistics right before the end of simulation
  Simulator::Schedule (sim_time-Seconds(0.05), &CustomProducer::StopApplication, this);
 
}

//Application is stopped
void
CustomProducer::StopApplication ()
{
  //results are printed only once
  if (GetNode()->GetId() == 0) {
  	for (int i=0; i< int(names.size ()); i++) 
  		std::cout << "Node " << names.at(i) << " received " << stats [ i ] << " interests\n";
  }
	
  //Cleanup ndn::App
  ndn::App::StopApplication ();
}

//Interest arrives
void
CustomProducer::OnInterest (Ptr<const ndn::Interest> interest)
{ 
  ndn::App::OnInterest (interest);
  Ptr<Node> producer = GetNode();
  std::string name = Names::FindName (producer);
  bool found = false;
  Time freshness = Seconds(1.0);  

  //hashing producer node name for signature
  //TODO: generate hash using SignatureSha256WithRsa algorithm of CCNx
  boost::hash<std::string> hash;

  //Emptry vector   
  if (names.empty ()) {
	names.push_back (name);
	stats [ 0 ]++;
	//std::cout << names.at(0) << " vs " << name << " " << stats[0] << "\n";
	counter++;
  }
  else {

  //Non-empty vector
  	for (int i=0; i< int(names.size ()); i++){
		if (name == names.at (i)) {
			stats [ i ]++;
			found = true; 
			std::cout << names.at(i) << " vs " << name << " " << stats[i] << "\n";
			break;
		}
  	}
  	if (!found) {  
   		names.push_back (name);
  		stats [ counter ]++;	
		//std::cout << names.at(counter) << " vs " << name << " " << stats[counter] << "\n";
		counter++;
  	}
  }

  NS_LOG_DEBUG ("Received Interest packet for " << interest->GetName ());
  
  //Preparing data packet
  Ptr<ndn::Data> data = Create<ndn::Data> (Create<Packet> (Payload));
  data->SetName (Create<ndn::Name> (interest->GetName ()));
  data->SetFreshness (freshness); 
  data->SetTimestamp (Simulator::Now ());
  //generate hash   
  std::size_t signature = hash(name); 
  data->SetSignature (signature);

  NS_LOG_DEBUG ("Sending Data packet for " << data->GetName ());  

  //Call trace
  m_transmittedDatas (data, this, m_face);

  m_face->ReceiveData (data); 
}

} //ns3
