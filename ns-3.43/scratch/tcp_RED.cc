/*
 * SPDX-License-Identifier: GPL-2.0-only
 */


// app = 15G the k = 65  lines = 10G avg per flow is 650 for tcp is 600
//
//
#include "ns3/netanim-module.h"
#include "ns3/core-module.h" //
#include "ns3/network-module.h" //
#include "ns3/internet-module.h" //
#include "ns3/point-to-point-module.h" //
#include "ns3/applications-module.h" //
#include "ns3/mobility-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NodeContainer coreSwitches, aggSwitches, edgeSwitches, hosts;
// ///////////////////////////START ANIMATION//////////////////////////
// void SetPosition(Ptr<Node> node, double x, double y) {
//     Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
//     mob->SetPosition(Vector(x, y, 0));
// }

// void AssignPositions(NodeContainer coreSwitches, NodeContainer aggSwitches,
//                      NodeContainer edgeSwitches, NodeContainer hosts,  uint32_t k, AnimationInterface anim) {
    
//     double xSpacing = 40.0; // Spacing between nodes
//     double ySpacing = 150.0; // Spacing between layers
//     double xoffset  = 150.0;   // offset each layer begins with
//     //AnimationInterface anim ("animation.xml");
//     // Core switches (Layer 1)
//     for (uint32_t i = 0; i < coreSwitches.GetN(); ++i) {
//         SetPosition(coreSwitches.Get(i), i * xSpacing + xoffset*1.5, ySpacing * 0);
//          uint32_t switchId = coreSwitches.Get(i)->GetId();
//         anim.UpdateNodeColor(switchId, 255, 255, 0);  // RGB for yellow
//         anim.UpdateNodeSize(switchId, 8.0, 8.0);      // Medium size
        
//     }

//     // Aggregation switches (Layer 2)
//     for (uint32_t i = 0; i < aggSwitches.GetN(); ++i) {
//         SetPosition(aggSwitches.Get(i), i * xSpacing + xoffset, ySpacing * 1);
//          uint32_t switchId = aggSwitches.Get(i)->GetId();
//         anim.UpdateNodeColor(switchId, 0, 0, 255);  // RGB for blue
//         anim.UpdateNodeSize(switchId, 8.0, 8.0);    // Medium size
//     }

//     // Edge switches (Layer 3)
//     for (uint32_t i = 0; i < edgeSwitches.GetN(); ++i) {
//         SetPosition(edgeSwitches.Get(i), i * xSpacing + xoffset , ySpacing*2);
//         uint32_t switchId = edgeSwitches.Get(i)->GetId();
//         anim.UpdateNodeColor(switchId, 0, 255, 0);  // RGB for green
//         anim.UpdateNodeSize(switchId, 8.0, 8.0);    // Medium size
//     }

//     // Hosts (Layer 4)
//     for (uint32_t i = 0; i < hosts.GetN(); ++i) {
//         SetPosition(hosts.Get(i), i * xSpacing, ySpacing * 3);
//         uint32_t hostId = hosts.Get(i)->GetId();
//         anim.UpdateNodeColor(hostId, 255, 0, 0);  // RGB for red
//         anim.UpdateNodeSize(hostId, 8.0, 8.0);    // Large size
//     }
// }
// ////////////////////////END ANIMATION////////////////////

int main(int argc, char *argv[]) {
    uint32_t k = 4; // Number of ports per switch
    CommandLine cmd;
    cmd.AddValue("k", "Number of ports per switch", k);
    cmd.Parse(argc, argv);




///////////////////////////////////// Fat tree ///////////////////////////////////////////////////////////////////////////////////////
    uint32_t numPods = k;
    uint32_t numCoreSwitches = (k / 2) * (k / 2);
    uint32_t numAggSwitches = (k * k) / 2;
    uint32_t numEdgeSwitches = numAggSwitches;
    uint32_t numHosts = k * k * k / 4;
    uint32_t threshold_k = 2666;

    

    //

    std::vector<NetDeviceContainer> link_vec;
    // DCTCP

    std::string tcpTypeId = "TcpReno";

    // Time configuration 
    Time flowStartupWindow = Seconds(1);
    Time convergenceTime = Seconds(3);
    Time measurementWindow = Seconds(1);
    bool enableSwitchEcn = false;
    Time progressInterval = MilliSeconds(100);
   // Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpTypeId));
    Time startTime = Seconds(0);
    Time stopTime = flowStartupWindow + convergenceTime + measurementWindow;

    //tcp config 
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2)); // for two packets return one ack
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));
    
    //RED config


    // Set default parameters for RED queue disc
    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(enableSwitchEcn));
    // ARED may be used but the queueing delays will increase; it is disabled
    // here because the SIGCOMM paper did not mention it
    // Config::SetDefault ("ns3::RedQueueDisc::ARED", BooleanValue (true));
    // Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
    Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(true));
    Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));  // TODO : need to set the packet size
    // Triumph and Scorpion switches used in DCTCP Paper have 4 MB of buffer
    // If every packet is 1500 bytes, 2666 packets can be stored in 4 MB
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize("2666p")));
    // DCTCP tracks instantaneous queue length only; so set QW = 1
    Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));
    // in the paper they have said the min max should be equal
    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(threshold_k));
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(threshold_k));


    TrafficControlHelper tchRed1;
    // MinTh = 20, MaxTh = 60 recommended in ACM SIGCOMM 2010 DCTCP Paper
    // This yields a target queue depth of 250us at 1 Gb/s
    tchRed1.SetRootQueueDisc("ns3::RedQueueDisc",
                             "LinkBandwidth",
                             StringValue("1Gbps"),
                             "LinkDelay",
                             StringValue("10us"),
                             "MinTh",
                             DoubleValue(threshold_k),
                             "MaxTh",
                             DoubleValue(threshold_k));


    // DCTCP

    // Nodes
    coreSwitches.Create(numCoreSwitches);
    aggSwitches.Create(numAggSwitches);
    edgeSwitches.Create(numEdgeSwitches);
    hosts.Create(numHosts);


    // for the 20, 60 thresholds we need to use 1Gbps
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Gbps")); 
    p2p.SetChannelAttribute("Delay", StringValue("10us"));


    // Internet Stack
    InternetStackHelper internet;
    internet.Install(coreSwitches);
    internet.Install(aggSwitches);
    internet.Install(edgeSwitches);
    internet.Install(hosts);


    
    // Core to Aggregation Links
    for (uint32_t pod = 0; pod < numPods; ++pod) {
        for (uint32_t i = 0; i < k / 2; ++i) {
            for (uint32_t j = 0; j < k / 2; ++j) {
                uint32_t coreIndex = i * (k / 2) + j;
                uint32_t aggIndex = pod * (k / 2) + i;
                NetDeviceContainer link = p2p.Install(coreSwitches.Get(coreIndex), aggSwitches.Get(aggIndex));
               
                link_vec.push_back(link);
                tchRed1.Install(link.Get(1));
                tchRed1.Install(link.Get(0));
            }
        }
    }

    // Aggregation to Edge Links
    for (uint32_t pod = 0; pod < numPods; ++pod) {
        for (uint32_t i = 0; i < k / 2; ++i) {
            for (uint32_t j = 0; j < k / 2; ++j) {
                uint32_t aggIndex = pod * (k / 2) + i;
                uint32_t edgeIndex = pod * (k / 2) + j;
                NetDeviceContainer link = p2p.Install(aggSwitches.Get(aggIndex), edgeSwitches.Get(edgeIndex));
                
                //DCTCP
                link_vec.push_back(link);
                tchRed1.Install(link.Get(1));
                tchRed1.Install(link.Get(0));
            }
        }
    }

    // Edge to Host Links
    for (uint32_t pod = 0; pod < numPods; ++pod) {
        for (uint32_t edgeIndex = pod * (k / 2); edgeIndex < (pod + 1) * (k / 2); ++edgeIndex) {
            for (uint32_t i = 0; i < k / 2; ++i) {
                uint32_t hostIndex = edgeIndex * (k / 2) + i;
                NetDeviceContainer link = p2p.Install(edgeSwitches.Get(edgeIndex), hosts.Get(hostIndex));
               
               
                //DCTCP 
                link_vec.push_back(link);
                tchRed1.Install(link.Get(1));
                tchRed1.Install(link.Get(0));
            }
        }
    }
    /////////////////////   


     // Assign IP Addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");

    
    for (uint32_t pod = 0; pod < link_vec.size(); ++pod) {    
        Ipv4InterfaceContainer interface = ipv4.Assign(link_vec[pod]);
        ipv4.NewNetwork();
    }
    //////////////////////////
    // Step 5: Enable Routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(coreSwitches);
    mobility.Install(aggSwitches);
    mobility.Install(edgeSwitches);
    mobility.Install(hosts);
    

///////////////////////////////dctcp application////////////////////////////////////////////
uint16_t basePort = 50000; // Base port for applications
std::vector<ApplicationContainer> sinkApps;
std::vector<ApplicationContainer> sourceApps;
uint32_t numFlows = 10; // Number of flows to simulate

for (uint32_t i = 0; i < numFlows; ++i) {
    uint16_t port = basePort + i;

    // Select sender and receiver hosts
    Ptr<Node> sender = hosts.Get(i % hosts.GetN());
    Ptr<Node> receiver = hosts.Get((hosts.GetN() - 1) % hosts.GetN());

    // Install PacketSink on the receiver
    Address sinkAddress(InetSocketAddress(receiver->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(receiver);
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(5.0));
    sinkApps.push_back(sinkApp);

    // Install on off on the sender
    OnOffHelper onOffHelper("ns3::TcpSocketFactory", sinkAddress);
    onOffHelper.SetAttribute("DataRate", StringValue("5Gbps")); // Sending rate
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1000)); // Packet size (bytes)
    ApplicationContainer sourceApp = onOffHelper.Install(sender);
    sourceApp.Start(Seconds(2.0));
    sourceApp.Stop(Seconds(5.0));

}

// Measure throughput at sinks

//window size = startsending_time - stop  in this case 5 - 2
for (uint32_t i = 0; i < sinkApps.size(); ++i) {
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApps[i].Get(0));
    Simulator::Schedule(Seconds(4.0), [sink, i]() {
        double throughput = sink->GetTotalRx() * 8.0 / (3.0 * 1e6); // Throughput in Mbps
        std::cout << "Flow " << i + 1 << ": Throughput = " << throughput << " Mbps" << std::endl;
    });
}



////////////////////////////////////////////////////////////////////////////////////////////



    ///////////////////////////////// Netanim picture settings//////////////////////////
//     AnimationInterface anim ("animation.xml");
//    // AssignPositions(coreSwitches, aggSwitches, edgeSwitches, hosts, k, anim);
//     double xSpacing = 40.0; // Spacing between nodes
//     double ySpacing = 150.0; // Spacing between layers
//     double xoffset  = 150.0;   // offset each layer begins with
//     //AnimationInterface anim ("animation.xml");
//     // Core switches (Layer 1)
//     for (uint32_t i = 0; i < coreSwitches.GetN(); ++i) {
//         SetPosition(coreSwitches.Get(i), i * xSpacing + xoffset*1.5, ySpacing * 0);
//          uint32_t switchId = coreSwitches.Get(i)->GetId();
//         anim.UpdateNodeColor(switchId, 255, 255, 0);  // RGB for yellow
//         anim.UpdateNodeSize(switchId, 8.0, 8.0);      // Medium size
        
//     }

//     // Aggregation switches (Layer 2)
//     for (uint32_t i = 0; i < aggSwitches.GetN(); ++i) {
//         SetPosition(aggSwitches.Get(i), i * xSpacing + xoffset, ySpacing * 1);
//          uint32_t switchId = aggSwitches.Get(i)->GetId();
//         anim.UpdateNodeColor(switchId, 0, 0, 255);  // RGB for blue
//         anim.UpdateNodeSize(switchId, 8.0, 8.0);    // Medium size
//     }

//     // Edge switches (Layer 3)
//     for (uint32_t i = 0; i < edgeSwitches.GetN(); ++i) {
//         SetPosition(edgeSwitches.Get(i), i * xSpacing + xoffset , ySpacing*2);
//         uint32_t switchId = edgeSwitches.Get(i)->GetId();
//         anim.UpdateNodeColor(switchId, 0, 255, 0);  // RGB for green
//         anim.UpdateNodeSize(switchId, 8.0, 8.0);    // Medium size
//     }

//     // Hosts (Layer 4)
//     for (uint32_t i = 0; i < hosts.GetN(); ++i) {
//         SetPosition(hosts.Get(i), i * xSpacing, ySpacing * 3);
//         uint32_t hostId = hosts.Get(i)->GetId();
//         anim.UpdateNodeColor(hostId, 255, 0, 0);  // RGB for red
//         anim.UpdateNodeSize(hostId, 8.0, 8.0);    // Large size
//     }
////////////////////////////////////////////////////////////////////////////////////////////////////

    Simulator::Stop(Seconds(5.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

/////////////////////////////////////////////////////////////
