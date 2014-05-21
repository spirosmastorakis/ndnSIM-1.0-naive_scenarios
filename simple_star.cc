/* simple_star.cc
					
                            consumer
                                |
                                | 2 Mbps
                        2 Mbps  |	 2 Mbps
              consumer - - - producer - - - consumer
                                |
                                | 2 Mbps
                                |
                            consumer

The number of spokes is specified by the input argument. Run this 
scenario directly: ./build/simple_star <number of spokes>

Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-star.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

int 
main (int argc, char *argv[])
{
  int spokes=0,i=0;	
  
  //default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("2Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("20ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("25"));

  //read input arguments. Default value is 4
  if (argc==2)
  	spokes=atoi(argv[1]);
  else {
	if (argc>2){
		printf("Unexpevted arguments. Please try again.\n");
		return 1;
	}
	else
		spokes=4;
  }

  //optional command-line parameters
  CommandLine cmd;
  cmd.Parse (argc, argv);

  //creating star topology
  PointToPointHelper p2p;
  PointToPointStarHelper star (spokes, p2p);
  star.BoundingBox(100,100,200,200);

  //NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ndnHelper.InstallAll ();

 //global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll ();

  //containers for the consumer/producer
  Ptr<Node> producer = star.GetHub ();
  NodeContainer consumerNodes;
  for (i=0;i<spokes;i++){
  	consumerNodes.Add (star.GetSpokeNode (i));
  }

  //NDN applications
  std::string prefix = "/prefix";

  ndn::AppHelper consumerHelper ("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix (prefix);
  consumerHelper.SetAttribute ("Frequency", StringValue ("100"));
  consumerHelper.Install (consumerNodes);

  ndn::AppHelper producerHelper ("ns3::ndn::Producer");
  producerHelper.SetPrefix (prefix);
  producerHelper.SetAttribute ("PayloadSize", StringValue("1024"));
  producerHelper.Install (producer);

  //add /prefix origins to ndn::GlobalRouter
  ndnGlobalRoutingHelper.AddOrigins (prefix, producer);

  //FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes ();

  Simulator::Stop (Seconds (10));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

