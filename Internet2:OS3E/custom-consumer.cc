//custom-consumer.cc

//Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

#include <algorithm>
#include <set>
#include "custom-consumer.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
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
  seq = 0;
  trasmitted_packets = 0; 
  RTO = Seconds(0.1);
  Retr_container.seqs.clear ();
  Retr_container.stamps.clear ();
  RTO_Events.clear ();
  //ndn::App
  ndn::App::StartApplication ();   
  //Interest sending policy based on lognormal distribution
  random_var = new LogNormalVariable (1.0 / frequency, 2 * 1.0 / frequency);
  CustomConsumer::SchedulePacket (); 
}

//Application is stopped
void
CustomConsumer::StopApplication ()
{
  //Cancel sendEvent  
  Simulator::Cancel (sendEvent);  

  //Cleanup ndn::App
  ndn::App::StopApplication (); 
}

void CustomConsumer::SchedulePacket ()
{
   if (!sendEvent.IsRunning ())
   	sendEvent = Simulator::Schedule(Seconds(random_var->GetValue ()), &CustomConsumer::SendInterest, this);
}

void
CustomConsumer::SendInterest ()
{  
  Retr_container.seqs.push_back (seq);
  Retr_container.stamps.push_back (Simulator::Now ());
  uint32_t this_seq = seq; 
  NS_LOG_DEBUG ("Pending seq number " << seq << " for node " << GetNode ()->GetId() );
  //Create and configure ndn::Interest
  Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
  ndn::Name prefix_without_seq = prefix;

  //Random sequence number 
  prefix.appendSeqNum (seq);
  //Increase sequence number
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  interest->SetNonce            (rand.GetValue ());
  interest->SetName             (prefix);
  interest->SetInterestLifetime (Seconds (1.0));
  
  NS_LOG_DEBUG ("Sending Interest packet for " << prefix); 
  
  //Schedule RTO event
  RTOevent= Simulator::Schedule(RTO, &CustomConsumer::RTO_Occured, this, this_seq);
  RTO_Events.push_back (RTOevent);
  //std::cout << "SIZE: " << RTO_Events.size() << " for Node" << GetNode()->GetId() << "\n";
  seq++;
  
  //Call trace
  m_transmittedInterests (interest, this, m_face);

  m_face->ReceiveInterest (interest);

  CustomConsumer::SchedulePacket (); 
}

//RTO
void 
CustomConsumer::RTO_Occured (uint32_t this_seq)
{
  std::vector<uint32_t>::iterator found_seq = std::search_n (Retr_container.seqs.begin (), Retr_container.seqs.end (), 1, this_seq);
  //std::cout << "Event running = " << RTO_Events.at(int (found_seq - Retr_container.seqs.begin ())).IsRunning () << "\n"; 
  if (found_seq != Retr_container.seqs.end ()) { 
	NS_LOG_DEBUG ("RTO occured for seq " << this_seq << " at Node " << GetNode()->GetId()) ;
  	//Double RTO
  	RTO += RTO;
	NS_LOG_DEBUG ("RTO doubled to " << RTO << " at node " << GetNode()->GetId());
  	int counter = 0;  
	trasmitted_packets = 0;
  	//Remove all sequence numbers <= seq awaiting for ACK
  	int size = int( Retr_container.seqs.size ());  
  	do {  
		if (Retr_container.seqs.at (counter) <= this_seq and !Retr_container.seqs.empty ()) {	
        		//std::cout << "seq_number " << Retr_container.seqs.at (counter) << "\n";
        		//std::cout << "sequence " << this_seq << "\n";
        		NS_LOG_DEBUG ("Erased sequence number for packet " << Retr_container.seqs.at (counter) << " at node " << GetNode()->GetId() );
  			//std::cout <<"Counter" << counter << "\n";
			Retr_container.seqs.erase (Retr_container.seqs.begin () + counter);
        		NS_LOG_DEBUG ("Erased timestamp for packet " << Retr_container.stamps.at (counter) << " at node " << GetNode()->GetId() );
  			Retr_container.stamps.erase (Retr_container.stamps.begin () + counter);
			Simulator::Cancel (RTO_Events.at(counter));
			//std::cout <<"Counter" << counter << "\n";
			size = int( Retr_container.seqs.size ());
			if (size > counter)
  				counter = size;
			else
				counter++;   
  		}
		else
			break;

	  } while (counter < size);
	seq = this_seq;
  }
}

//Data arrives
void
CustomConsumer::OnData (Ptr<const ndn::Data> contentObject)
{
  uint32_t received_seq = contentObject->GetName ().get (-1).toSeqNum ();
  //std::cout << received_seq << "\n";
  NS_LOG_DEBUG ("Receiving Data packet for " << contentObject->GetName ());
  std::vector<uint32_t>::iterator pending_seq = search_n (Retr_container.seqs.begin (), Retr_container.seqs.end (), 1, received_seq);
  if (pending_seq == Retr_container.seqs.end ()) {
	//Data packet arrived after timeout
	NS_LOG_DEBUG ("Data for " << contentObject->GetName () << " must be discarded"); 
	seq = received_seq;
	NS_LOG_DEBUG ("Performing retrasmission for " << seq);
  }
  else {
	if (int (pending_seq - Retr_container.seqs.begin ()) != 0) {
		//Out of order data packet received
		//std::cout << "Wrong order\n";
  		//std::cout << (int (pending_seq - Retr_container.seqs.begin ())) << "\n" ;
		//std::cout << "Node: " << GetNode()->GetId()<< "\n" ;
		NS_LOG_DEBUG ("Data for " << contentObject->GetName () << " must be discarded because it is out of order");
		for (int i = 0 ; i <= int(pending_seq - Retr_container.seqs.begin ()); i++) {
			Simulator::Cancel (RTO_Events.at(i));
			Retr_container.seqs.erase (Retr_container.seqs.begin () + i);
			Retr_container.stamps.erase (Retr_container.stamps.begin () + i);
		}
		trasmitted_packets = 0;
		seq = received_seq;
		NS_LOG_DEBUG ("Performing retrasmission for data " << contentObject->GetName ()  << " and sequence number " << seq << " at Node " << GetNode()->GetId());
	}	
	else { 
		//Data packet trasmitted successfully 
		//std::cout << " Node "<< GetNode()->GetId() <<"\n";
		//std::cout << "seq num " << int(pending_seq - Retr_container.seqs.begin ()) << "\n";

		//Erase sequence number, timestamps and cancel timeout events
  		Simulator::Cancel (RTO_Events.at(int(pending_seq - Retr_container.seqs.begin ())));
		Retr_container.seqs.erase (Retr_container.seqs.begin () + int(pending_seq - Retr_container.seqs.begin ()));
		Retr_container.stamps.erase (Retr_container.stamps.begin () + int(pending_seq - Retr_container.seqs.begin ()));			
		//Increase number of consecutive packets that have trasmitted successfully
		trasmitted_packets++;
		//Every three consecutive packets that have been trasmitted successfully, RTO is decreased
		if (trasmitted_packets == 3) {
			trasmitted_packets = 0;	
			double _RTO = RTO.ToDouble (Time::Unit (4));
			_RTO = 0.75 * _RTO;
			RTO = RTO.FromDouble (_RTO,Time::Unit (4));
			NS_LOG_DEBUG ("RTO decreased to " << RTO << " at node " << GetNode()->GetId());
		}
		NS_LOG_DEBUG ("Acceptable data for " << contentObject->GetName () << " with sequence number " << received_seq << " at Node "<< GetNode()->GetId() );
  	}
  } 
}

} 
