//disjoint-paths.cc
/*
	   - - - - - router - - - - - router - - - - - 	
      |                                           |
	producer                                    consumer
      |                                           |
	   - - - - - router - - - - - router - - - - - 

Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  AnnotatedTopologyReader topologyReader ("", 1);
  topologyReader.SetFileName ("/ndnSIM/ns-3/scratch/disjoint_paths.txt");
  topologyReader.Read ();

  //CCNx stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.InstallAll ();

  //global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();

  //consumer/producer
  Ptr<Node> consumer = Names::Find<Node> ("consumer");
  Ptr<Node> producer = Names::Find<Node> ("producer");
  
  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerBatches");
  consumerHelper.SetPrefix ("/producer");
  consumerHelper.SetAttribute ("Batches", StringValue ("10"));
  consumerHelper.Install (consumer);

  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));  

  //Register /root prefix with global routing controller and
  //install producer that will satisfy Interests in /root namespace
  ccnxGlobalRoutingHelper.AddOrigins ("/producer", producer);
  producerHelper.SetPrefix ("/producer");
  producerHelper.Install (producer)
    .Start (Seconds (9));

  //FIBs
  ccnxGlobalRoutingHelper.CalculateRoutes ();

  Simulator::Stop (Seconds (20.0));
  
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
