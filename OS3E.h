//OSE3.h
//Author: Spyridon Mastorakis <spiros[dot]mastorakis[at]gmail[dot]com>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

void 
register_prefix (int i, Ptr<Node> producers [ ], ndn::GlobalRoutingHelper Helper, NodeContainer topology);
