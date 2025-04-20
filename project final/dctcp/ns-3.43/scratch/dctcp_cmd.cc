#include "ns3/flow-monitor-module.h"
#include "ns3/core-module.h" //
#include "ns3/network-module.h" //
#include "ns3/internet-module.h" //
#include "ns3/point-to-point-module.h" //
#include "ns3/applications-module.h" //
#include "ns3/mobility-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/log.h"
#include <fstream>
#include <iostream>
#include <random> 
#include <unordered_set> 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MySimulation");

NodeContainer coreSwitches, aggSwitches, edgeSwitches, hosts;


//-----------global variables for the coommand line----------
int mode = 0;   
                //mode 0 = all senders are known and send to a known reciever (only on ereciever)
                //mode 1 = random senders and random reciever (only one reciever)
                //mode 2 = random senders and random recievers
int k = 4;      //num of ports per switch
std::string p2p_DataRate = "1Gbps";     //point to point data rate
std::string p2p_Delay = "10us";     //point to point delay
uint32_t num_of_flows = 100;
int app_packet_size = 1000;
std::string app_data_rate = "5Gbps";
double simulation_stop_time = 15;
double flowmonitor_start_time = 0;
std::string protocol = "TcpDctcp";
uint32_t swift_mode = 0;


//------------generate one random number-----------
int GetRandomNumber(int n) 
{
    std::random_device rd;   
    std::mt19937 gen(rd());     
    std::uniform_int_distribution<> distrib(0, n); // Range [0, n]
    return distrib(gen);       
}
//--------------------------------------


//---------func to generate random numbers for the random connections-----------
std::vector<int> GetRandomNumbers(int n, int x, int q) 
{
    std::random_device rd;     
    std::mt19937 gen(rd());    
    std::uniform_int_distribution<> distrib(0, x); // Range [0, x]

    std::vector<int> result;
    for (int i = 0; i < n; ++i) 
    {
        int num;
        do {
            num = distrib(gen);
        } while (num == q); // Skip 'q' if you still want to avoid it
        result.push_back(num);
    }
    return result;
}
//----------------------------------------------------------------



// Define a pair of integers for convenience
using Pair = std::pair<int, int>;

//-------------this func generatees pairs of numbers for the random connections-----
std::vector<Pair> GetRandomPairs(int n, int x) 
{
    if (n <= 0 || x < 1) { // x must be at least 1 to ensure two distinct numbers
        throw std::invalid_argument("n must be > 0 and x must be >= 1.");
    }
    std::random_device rd;    
    std::mt19937 gen(rd());    
    std::uniform_int_distribution<> dist(0, x); // Range [0, x]
    std::vector<Pair> randomPairs;

    for (int i = 0; i < n; ++i) 
    {
        int first = dist(gen); 
        int second;
        do 
        {
            second = dist(gen);
        } while (second == first);

        randomPairs.emplace_back(first, second);
    }
    return randomPairs;
}
//----------------------------------------------------------------



//-----------this function if to track the window size for a connection--------------
std::ofstream cwndFile("scratch/swift_cwnd.txt", std::ios::out);

void CwndTracer(uint32_t oldCwnd, uint32_t newCwnd) 
{
    double time = Simulator::Now().GetSeconds(); // Get simulation time
    cwndFile << time << " " << newCwnd << std::endl; // Write time and cwnd to file
    std::cout << "[DEBUG] Time: " << time << "s, CWND Changed: " << oldCwnd << " -> " << newCwnd << std::endl;
}

void Track_Flow(Ptr<TcpSocketBase> socket)
{
    socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndTracer));
}







int main(int argc, char *argv[]) 
{
    //---------------------this part is for the command line implementation---------------------------
    CommandLine cmd;
    cmd.AddValue("mode", "chose a mode", mode);
    cmd.AddValue("k", "Number of ports per switch", k);
    cmd.AddValue("p2p_DataRate", "point to point data rate", p2p_DataRate);
    cmd.AddValue("p2p_Delay", "point to point delay", p2p_Delay);
    cmd.AddValue("num_of_flows", "Number of flows", num_of_flows);
    cmd.AddValue("app_packet_size", "application packet size", app_packet_size);
    cmd.AddValue("app_data_rate", "application data rate", app_data_rate);
    cmd.AddValue("flowmonitor_start_time", "flowmonitor time", flowmonitor_start_time);
    cmd.AddValue("simulation_stop_time", "simulation stop time", simulation_stop_time);
    cmd.AddValue("protocol", "choose the protocol to run on the topology", protocol);
    cmd.AddValue("swift_mode", "choose the target delay", swift_mode);
    cmd.Parse(argc, argv);
    //----------------------------command line implementation ends here---------------------------------

    std::clog.rdbuf(std::cout.rdbuf());

    NS_LOG_INFO("Starting the simulation...");
    
    //here starts the fat tree implementation
    uint32_t numPods = k;
    uint32_t numCoreSwitches = (k / 2) * (k / 2);
    uint32_t numAggSwitches = (k * k) / 2;
    uint32_t numEdgeSwitches = numAggSwitches;
    uint32_t numHosts = k * k * k / 4;
    uint32_t threshold_k = 250;   ////// needs to be checked
    
    std::vector<NetDeviceContainer> link_vec;
    // Swift
    std::string tcpTypeId = protocol;
    // Time configuration 
    bool enableSwitchEcn;
    bool enable_swift;
    
   
    enableSwitchEcn = true;
    enable_swift = false;
    threshold_k = 250;
    
    
   
    //tcp config 
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + tcpTypeId));
    Config::SetDefault("ns3::TcpSocketState::MaxPacingRate", DataRateValue(DataRate("1Gb/s")));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1500));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1)); // for two packets return one ack
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));
    
    //RED config

    // Set default parameters for RED queue disc
    Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(enableSwitchEcn));
    Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (false));
    Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(false));
    Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(1500));
    // Triumph and Scorpion switches used in DCTCP Paper have 4 MB of buffer
    // If every packet is 1500 bytes, 2666 packets can be stored in 4 MB
    Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize("2666p")));
    // DCTCP tracks instantaneous queue length only; so set QW = 1
    Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));
    // in the paper they have said the min max should be equal
    Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(threshold_k));
    Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(threshold_k));


    TrafficControlHelper tchRed1;
    tchRed1.SetRootQueueDisc("ns3::RedQueueDisc",
                             "LinkBandwidth",
                             StringValue(p2p_DataRate),
                             "LinkDelay",
                             StringValue(p2p_Delay),
                             "MinTh",
                             DoubleValue(threshold_k),
                             "MaxTh",
                             DoubleValue(threshold_k));

    // Nodes
    coreSwitches.Create(numCoreSwitches);
    aggSwitches.Create(numAggSwitches);
    edgeSwitches.Create(numEdgeSwitches);
    hosts.Create(numHosts);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(p2p_DataRate)); 
    p2p.SetChannelAttribute("Delay", StringValue(p2p_Delay));

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
                link_vec.push_back(link);
                tchRed1.Install(link.Get(1));
                tchRed1.Install(link.Get(0));
            }
        }
    }

     // Assign IP Addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");

    for (uint32_t pod = 0; pod < link_vec.size(); ++pod) {    
        Ipv4InterfaceContainer interface = ipv4.Assign(link_vec[pod]);
        ipv4.NewNetwork();
    }
   
    // Step 5: Enable Routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(coreSwitches);
    mobility.Install(aggSwitches);
    mobility.Install(edgeSwitches);
    mobility.Install(hosts);
    

    //---------------------here starts the implementation of the connections between servers------------------
    uint16_t basePort = 50000; // Base port for applications
    std::vector<ApplicationContainer> sinkApps;
    std::vector<ApplicationContainer> sourceApps;

    //mode 0 is when the reciever is the last and the senders are from the beginning
    if(mode == 0) 
    {
        int random_window_to_track  = GetRandomNumber(num_of_flows-1); 
        for (uint32_t i = 0; i < num_of_flows; ++i) 
        {
            uint16_t port = basePort + i;
            if ((i % hosts.GetN()) == ((hosts.GetN() - 1) % hosts.GetN()))
            {
                continue;
            }
            Ptr<Node> sender = hosts.Get(i % hosts.GetN());
            Ptr<Node> receiver = hosts.Get((hosts.GetN() - 1) % hosts.GetN());


            // Install PacketSink on the receiver
            Address sinkAddress(InetSocketAddress(receiver->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port));
            PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
            ApplicationContainer sinkApp = sinkHelper.Install(receiver);
            sinkApp.Start(Seconds(1.0));
            sinkApp.Stop(Seconds(simulation_stop_time));
            sinkApps.push_back(sinkApp);

            //  Install on off on the sender
            OnOffHelper onOffHelper("ns3::TcpSocketFactory", sinkAddress);
            onOffHelper.SetAttribute("DataRate", StringValue(app_data_rate)); // Sending rate
            onOffHelper.SetAttribute("PacketSize", UintegerValue(app_packet_size)); // Packet size (bytes)
            ApplicationContainer sourceApp = onOffHelper.Install(sender);
            sourceApp.Start(Seconds(2.0));
            sourceApp.Stop(Seconds(simulation_stop_time - 1));

            if (/*i == random_window_to_track*/1 && i == 0) { 
                Ptr<Application> app = sourceApp.Get(0); // Get the OnOffApplication instance
                Ptr<OnOffApplication> onOffApp = DynamicCast<OnOffApplication>(app);
                
                if (onOffApp) {
                    Simulator::Schedule(Seconds(2.01), [onOffApp]() {
                        Ptr<Socket> onOffSocket = onOffApp->GetSocket();
                        Ptr<TcpSocketBase> tcpSocketBase = DynamicCast<TcpSocketBase>(onOffSocket);
                        
                        if (tcpSocketBase) {
                            std::cout << "[DEBUG] Attaching cwnd tracer to connection 0 (Sender Node " << onOffApp->GetNode()->GetId() << ")" << std::endl;
                            Track_Flow(tcpSocketBase);  
                        } else {
                            std::cerr << "[ERROR] Could not cast OnOff socket to TcpSocketBase!" << std::endl;
                        }
                    });
                } else {
                    std::cerr << "[ERROR] Could not retrieve OnOffApplication instance!" << std::endl;
                }
            }
        }
    }
    
    // mode 1 is when the reciever is random and the senders are random
    else if(mode == 1)
    {  
        int reciever_number = GetRandomNumber(hosts.GetN()-1);
        Ptr<Node> receiver = hosts.Get(reciever_number);
        std::vector<int> randomNumbers = GetRandomNumbers(num_of_flows, hosts.GetN()-1, reciever_number);  
        for(int i=0 ;i<randomNumbers.size() ;i++)
        {
            Ptr<Node> sender = hosts.Get(randomNumbers[i]);
            uint16_t port = basePort + i;

            // Install PacketSink on the receiver
            Address sinkAddress(InetSocketAddress(receiver->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port));
            PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
            ApplicationContainer sinkApp = sinkHelper.Install(receiver);
            sinkApp.Start(Seconds(1.0));
            sinkApp.Stop(Seconds(simulation_stop_time));
            sinkApps.push_back(sinkApp);

            //  Install on off on the sender
            OnOffHelper onOffHelper("ns3::TcpSocketFactory", sinkAddress);
            onOffHelper.SetAttribute("DataRate", StringValue(app_data_rate)); // Sending rate
            onOffHelper.SetAttribute("PacketSize", UintegerValue(app_packet_size)); // Packet size (bytes)
            ApplicationContainer sourceApp = onOffHelper.Install(sender);
            sourceApp.Start(Seconds(2.0));
            sourceApp.Stop(Seconds(simulation_stop_time-1));


            if (/*i == random_window_to_track*/1 && i == 1) { 
                Ptr<Application> app = sourceApp.Get(0); // Get the OnOffApplication instance
                Ptr<OnOffApplication> onOffApp = DynamicCast<OnOffApplication>(app);
                
                if (onOffApp) {
                    Simulator::Schedule(Seconds(2.01), [onOffApp]() {
                        Ptr<Socket> onOffSocket = onOffApp->GetSocket();
                        Ptr<TcpSocketBase> tcpSocketBase = DynamicCast<TcpSocketBase>(onOffSocket);
                        
                        if (tcpSocketBase) {
                            std::cout << "[DEBUG] Attaching cwnd tracer to connection 0 (Sender Node " << onOffApp->GetNode()->GetId() << ")" << std::endl;
                            Track_Flow(tcpSocketBase);  
                        } else {
                            std::cerr << "[ERROR] Could not cast OnOff socket to TcpSocketBase!" << std::endl;
                        }
                    });
                } else {
                    std::cerr << "[ERROR] Could not retrieve OnOffApplication instance!" << std::endl;
                }
            }
        }
    }
    
    else if(mode == 2)
    {
        std::vector<Pair> random_pairs = GetRandomPairs(num_of_flows, hosts.GetN()-1);
        for(int i=0 ; i<random_pairs.size(); i++)
        {
            uint16_t port = basePort + i;
            Ptr<Node> receiver = hosts.Get(random_pairs[i].first);
            Ptr<Node> sender = hosts.Get(random_pairs[i].second);

            // Install PacketSink on the receiver
            Address sinkAddress(InetSocketAddress(receiver->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port));
            PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
            ApplicationContainer sinkApp = sinkHelper.Install(receiver);
            sinkApp.Start(Seconds(1.0));
            sinkApp.Stop(Seconds(simulation_stop_time));
            sinkApps.push_back(sinkApp);

            //  Install on off on the sender
            OnOffHelper onOffHelper("ns3::TcpSocketFactory", sinkAddress);
            onOffHelper.SetAttribute("DataRate", StringValue(app_data_rate)); // Sending rate
            onOffHelper.SetAttribute("PacketSize", UintegerValue(app_packet_size)); // Packet size (bytes)
            ApplicationContainer sourceApp = onOffHelper.Install(sender);
            sourceApp.Start(Seconds(2.0));
            sourceApp.Stop(Seconds(simulation_stop_time -1));


            if (/*i == random_window_to_track*/1 && i == 0) { 
                Ptr<Application> app = sourceApp.Get(0); // Get the OnOffApplication instance
                Ptr<OnOffApplication> onOffApp = DynamicCast<OnOffApplication>(app);
                
                if (onOffApp) {
                    Simulator::Schedule(Seconds(2.01), [onOffApp]() {
                        Ptr<Socket> onOffSocket = onOffApp->GetSocket();
                        Ptr<TcpSocketBase> tcpSocketBase = DynamicCast<TcpSocketBase>(onOffSocket);
                        
                        if (tcpSocketBase) {
                            std::cout << "[DEBUG] Attaching cwnd tracer to connection 0 (Sender Node " << onOffApp->GetNode()->GetId() << ")" << std::endl;
                            Track_Flow(tcpSocketBase);  
                        } else {
                            std::cerr << "[ERROR] Could not cast OnOff socket to TcpSocketBase!" << std::endl;
                        }
                    });
                } else {
                    std::cerr << "[ERROR] Could not retrieve OnOffApplication instance!" << std::endl;
                }
            }
        }
    }
//-----------------------here ends the implementation of the connections between servers--------------------

   
//--------------------------flow monitor---------------------------
    int j=0;
    float AvgThroughput = 0;
    Time Delay = Seconds(0);
    uint32_t ReceivedPackets = 0;
    FlowMonitorHelper flowmon;
    float packet_loss_rate = 0;

    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    Simulator::Schedule(Seconds(flowmonitor_start_time), [monitor]() {
    monitor->StartRightNow();
    });


    Simulator::Schedule(Seconds(simulation_stop_time + 10), [monitor]() {
    monitor->StopRightNow();
    });


    Simulator::Stop(Seconds(simulation_stop_time + 10));

    //LogComponentEnable("TcpSwift", ns3::LOG_LEVEL_DEBUG);
    //LogComponentEnable("TcpSocketBase", ns3::LOG_LEVEL_DEBUG);
    
    Simulator::Run();

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    for (auto iter = stats.begin (); iter != stats.end (); ++iter) 
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 

        packet_loss_rate += (iter->second.txPackets - iter->second.rxPackets) * 100.0 / static_cast<double>(iter->second.txPackets);
        Delay = Delay + (iter->second.delaySum);
        ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
        j = j + 1;
        if (j == num_of_flows)
        {
            break;
        }
        
    }   
        packet_loss_rate = packet_loss_rate / j;
        AvgThroughput = AvgThroughput/j;

        std::cout << "--------Total Results of the simulation----------"<<std::endl;
        std::cout << "Average Throughput =" << AvgThroughput << "Kbps"<< std::endl;
        std::cout << "End to End Delay =" << Delay.GetSeconds()/ReceivedPackets<< std::endl;
        std::cout << "Average packet loss rate = " << packet_loss_rate <<"% " <<std::endl;

        //------------here starts the writing into the output file implementation---------

        std::string filename;
   
        filename = "scratch/results_dctcp.txt";
       
        std::ofstream outputFile(filename, std::ios::app);
         if (!outputFile.is_open()) 
         {
            std::cout << "Error: Could not open file output.txt for appending" << std::endl;
            return 1;
         }
         outputFile << "flows = " << num_of_flows << "   ";
         outputFile << "Throughput = " <<AvgThroughput << " Kbps   ";
         outputFile<< "Delay = " << Delay.GetSeconds()/ReceivedPackets<<"   ";
         outputFile << "time = " << simulation_stop_time << "   ";
         outputFile << "packet loss = " << packet_loss_rate <<"%   " << "K = " << k <<"   mode = "<< mode << std::endl;
         outputFile.close();
         //--------------here ends the writing into the output file implementation---------

        Simulator::Destroy();
        cwndFile.close();
    return 0;
}
