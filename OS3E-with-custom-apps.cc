//OS3E-with-custom-apps.cc

/*

Enabling NDN across Internet2/OS3E!

Custom app was implemented for producers. Statistics are kept.
TODO: Implement custom app for consumers
 
Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

*/

#include <time.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

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
  topologyReader.SetFileName ("/ndnSIM/ns-3/src/ndnSIM/examples/OS3E.txt");
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
  
  //define simulation time
  Time sim_time = Seconds(2.0);
  int random_producer=0;
  srand (time (NULL));
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
      ndn::AppHelper producerHelper ("CustomApp");
      producerHelper.SetAttribute ("DataSize", StringValue ("1024"));
      std::stringstream parameter;
      parameter << int(topo.GetN());
      producerHelper.SetAttribute ("Node number", StringValue (parameter.str ()));
      producerHelper.SetAttribute ("Sim time", TimeValue (sim_time)); 
      register_prefix(i, producers, ccnxGlobalRoutingHelper, topo); 
      producerHelper.SetPrefix ("/" + Names::FindName(producers[i]));
      producerHelper.Install (producers[i]);
    }

  //FIBs
  ccnxGlobalRoutingHelper.CalculateRoutes ();

  Simulator::Stop (sim_time);

  ndn::L3AggregateTracer::InstallAll ("aggregate-trace.txt", Seconds (0.5));
  ndn::L3RateTracer::InstallAll ("rate-trace.txt", Seconds (0.5));
  
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}