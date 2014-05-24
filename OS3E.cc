//OS3E.cc

/*

Running NDN over Internet2/OS3E!

TODO: Implement custom apps for consumers and producers
 
Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

*/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "OS3E.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

void register_prefix (int this_producer, Ptr<Node> producers[ ], ndn::GlobalRoutingHelper Helper, NodeContainer topology){
  //Register prefixes with global routing controller
  for (int i=0; i< int(topology.GetN ()); i++) {
	if (producers[i] != producers[this_producer])
  		Helper.AddOrigins ("/" + Names::FindName(producers[i]), producers[this_producer]); 
  }	
}

int main (int argc, char *argv[]){

  CommandLine cmd;
  cmd.Parse (argc, argv);

  //Parsing Internet2/OS3E topology
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("/ndnSIM/ns-3/scratch/OS3E.txt");
  topologyReader.Read ();
  NodeContainer topo = topologyReader.GetNodes();

  //CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ccnxHelper.InstallAll ();

  //Global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();

  //Install one consumer and one producer on each node
  int counter=0;
  std::string name = "";
  Ptr<Node> consumers[int(topo.GetN())], producers[int(topo.GetN())];
  for (NodeContainer::Iterator i= topo.Begin ();i != topo.End ();i++){
	name = Names::FindName(*i);
  	consumers[counter] = (*i);
  	producers[counter] = (*i);
	counter++;
  }

  int random_producer=0;
  //Unique prefixes for each consumer
  for (int i = 0; i < int(topo.GetN ()); i++)
    {
      ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
      consumerHelper.SetAttribute ("Frequency", StringValue ("5"));
      //Each consumer will express random data /producer[random]/<seq-no>
      random_producer = rand() % topo.GetN ();
      consumerHelper.SetPrefix ("/"+ Names::FindName(producers[random_producer]));
      consumerHelper.Install (consumers[i]);
    }

  for (int i = 0; i < int(topo.GetN ()); i++)
    {
      ndn::AppHelper producerHelper ("ns3::ndn::Producer");
      producerHelper.SetAttribute ("PayloadSize", StringValue ("1024"));
      register_prefix(i, producers, ccnxGlobalRoutingHelper, topo);
      //ccnxGlobalRoutingHelper.AddOrigins ("/" + Names::FindName(producers[i]) , producers[i]);
      producerHelper.SetPrefix ("/" + Names::FindName(producers[i]));
      producerHelper.Install (producers[i]);
    }

  //FIBs
  ccnxGlobalRoutingHelper.CalculateRoutes ();

  Simulator::Stop (Seconds (10.0));

  ndn::L3AggregateTracer::InstallAll ("aggregate-trace.txt", Seconds (0.5));
  ndn::L3RateTracer::InstallAll ("rate-trace.txt", Seconds (0.5));
  
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

