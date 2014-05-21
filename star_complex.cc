//star-complex.cc
/*

                        consumer
                            |
                            |
                        router
                            |
                            |
consumer - - router - - producer - - router - - consumer
                            |
                            |
                        router
                            |
                            |
                        consumer


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

  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("/ndnSIM/ns-3/scratch/topo-star-complex.txt");
  topologyReader.Read ();

  //CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ccnxHelper.InstallAll ();

  //Global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();

  //containers for the consumer/producer
  Ptr<Node> consumers[4] = { Names::Find<Node> ("consumer1"), Names::Find<Node> ("consumer2"),
                             Names::Find<Node> ("consumer3"), Names::Find<Node> ("consumer4") };
  Ptr<Node> producer = Names::Find<Node> ("producer1");

  for (int i = 0; i < 4; i++)
    {
      ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
      consumerHelper.SetAttribute ("Frequency", StringValue ("100")); // 100 interests a second

      //each consumer expresses unique interests /root/<leaf-name>/<seq-no>
      consumerHelper.SetPrefix ("/producer1/");
      consumerHelper.Install (consumers[i]);
    }
    
  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));  

  //register /root prefix with global routing controller and
  //install producer that will satisfy Interests in /root namespace
  ccnxGlobalRoutingHelper.AddOrigins ("/producer1", producer);
  producerHelper.SetPrefix ("/producer1");
  producerHelper.Install (producer);

  //FIBs
  ccnxGlobalRoutingHelper.CalculateRoutes ();

  Simulator::Stop (Seconds (10.0));

  ndn::L3AggregateTracer::InstallAll ("aggregate-trace.txt", Seconds (0.5));
  ndn::L3RateTracer::InstallAll ("rate-trace.txt", Seconds (0.5));
  
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
