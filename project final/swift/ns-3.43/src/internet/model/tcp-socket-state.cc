#include "tcp-socket-state.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpSocketState);

TypeId
TcpSocketState::GetTypeId()
{
 static TypeId tid =
     TypeId("ns3::TcpSocketState")
         .SetParent<Object>()
         .SetGroupName("Internet")
         .AddConstructor<TcpSocketState>()
         .AddAttribute("EnablePacing",
                       "Enable Pacing",
                       BooleanValue(false),
                       MakeBooleanAccessor(&TcpSocketState::m_pacing),
                       MakeBooleanChecker())
         .AddAttribute("MaxPacingRate",
                       "Set Max Pacing Rate",
                       DataRateValue(DataRate("4Gb/s")),
                       MakeDataRateAccessor(&TcpSocketState::m_maxPacingRate),
                       MakeDataRateChecker())
         .AddAttribute("PacingSsRatio",
                       "Percent pacing rate increase for slow start conditions",
                       UintegerValue(200),
                       MakeUintegerAccessor(&TcpSocketState::m_pacingSsRatio),
                       MakeUintegerChecker<uint16_t>())
         .AddAttribute("PacingCaRatio",
                       "Percent pacing rate increase for congestion avoidance conditions",
                       UintegerValue(120),
                       MakeUintegerAccessor(&TcpSocketState::m_pacingCaRatio),
                       MakeUintegerChecker<uint16_t>())
         .AddAttribute("PaceInitialWindow",
                       "Perform pacing for initial window of data",
                       BooleanValue(false),
                       MakeBooleanAccessor(&TcpSocketState::m_paceInitialWindow),
                       MakeBooleanChecker())
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////SWIFT MODIFICATION///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         .AddAttribute ("TimeofLastDecrease","Time of last decrease",
                          EmptyAttributeValue (),
                         MakeTimeAccessor (&TcpSocketState::m_t_last_decrease),
                         MakeTimeChecker ())
         .AddAttribute ("RetransmitCount", "Retransmit Count",
                         UintegerValue (0),
                         MakeUintegerAccessor (&TcpSocketState::m_retransmit_count),
                         MakeUintegerChecker<uint32_t> ())
         .AddAttribute ("max_cwnd", "Maximum Congestion Window",
                         UintegerValue (INIT_MAX),
                         MakeUintegerAccessor (&TcpSocketState::m_max_cwnd),
                         MakeUintegerChecker<uint32_t> ())
         .AddAttribute ("min_cwnd", "Maximum Congestion Window",
                         UintegerValue (INIT_MIN),
                         MakeUintegerAccessor (&TcpSocketState::m_min_cwnd),
                         MakeUintegerChecker<uint32_t> ())
         .AddAttribute ("max_mdf", "Limit on increase",
                         UintegerValue (1),
                         MakeUintegerAccessor (&TcpSocketState::m_max_mdf),
                         MakeUintegerChecker<uint32_t> ())
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         .AddTraceSource("PacingRate",
                         "The current TCP pacing rate",
                         MakeTraceSourceAccessor(&TcpSocketState::m_pacingRate),
                         "ns3::TracedValueCallback::DataRate")
         .AddTraceSource("CongestionWindow",
                         "The TCP connection's congestion window",
                         MakeTraceSourceAccessor(&TcpSocketState::m_cWnd),
                         "ns3::TracedValueCallback::Uint32")
         .AddTraceSource("CongestionWindowInflated",
                         "The TCP connection's inflated congestion window",
                         MakeTraceSourceAccessor(&TcpSocketState::m_cWndInfl),
                         "ns3::TracedValueCallback::Uint32")
         .AddTraceSource("SlowStartThreshold",
                         "TCP slow start threshold (bytes)",
                         MakeTraceSourceAccessor(&TcpSocketState::m_ssThresh),
                         "ns3::TracedValueCallback::Uint32")
         .AddTraceSource("CongState",
                         "TCP Congestion machine state",
                         MakeTraceSourceAccessor(&TcpSocketState::m_congState),
                         "ns3::TracedValueCallback::TcpCongState")
         .AddTraceSource("EcnState",
                         "Trace ECN state change of socket",
                         MakeTraceSourceAccessor(&TcpSocketState::m_ecnState),
                         "ns3::TracedValueCallback::EcnState")
         .AddTraceSource("HighestSequence",
                         "Highest sequence number received from peer",
                         MakeTraceSourceAccessor(&TcpSocketState::m_highTxMark),
                         "ns3::TracedValueCallback::SequenceNumber32")
         .AddTraceSource("NextTxSequence",
                         "Next sequence number to send (SND.NXT)",
                         MakeTraceSourceAccessor(&TcpSocketState::m_nextTxSequence),
                         "ns3::TracedValueCallback::SequenceNumber32")
         .AddTraceSource("BytesInFlight",
                         "The TCP connection's congestion window",
                         MakeTraceSourceAccessor(&TcpSocketState::m_bytesInFlight),
                         "ns3::TracedValueCallback::Uint32")
         .AddTraceSource("RTT",
                         "Smoothed RTT",
                         MakeTraceSourceAccessor(&TcpSocketState::m_srtt),
                         "ns3::TracedValueCallback::Time")
         .AddTraceSource("LastRTT",
                         "RTT of the last (S)ACKed packet",
                         MakeTraceSourceAccessor(&TcpSocketState::m_lastRtt),
                         "ns3::TracedValueCallback::Time");
 return tid;
}

TcpSocketState::TcpSocketState(const TcpSocketState& other)
 : Object(other),
   m_cWnd(other.m_cWnd),
   m_ssThresh(other.m_ssThresh),
   m_initialCWnd(other.m_initialCWnd),
   m_initialSsThresh(other.m_initialSsThresh),
   m_segmentSize(other.m_segmentSize),
   m_lastAckedSeq(other.m_lastAckedSeq),
   m_congState(other.m_congState),
   m_ecnState(other.m_ecnState),
   m_highTxMark(other.m_highTxMark),
   m_nextTxSequence(other.m_nextTxSequence),
   m_rcvTimestampValue(other.m_rcvTimestampValue),
   m_rcvTimestampEchoReply(other.m_rcvTimestampEchoReply),
   m_pacing(other.m_pacing),
   m_maxPacingRate(other.m_maxPacingRate),
   m_pacingRate(other.m_pacingRate),
   m_pacingSsRatio(other.m_pacingSsRatio),
   m_pacingCaRatio(other.m_pacingCaRatio),
   m_paceInitialWindow(other.m_paceInitialWindow),
   m_minRtt(other.m_minRtt),
   m_bytesInFlight(other.m_bytesInFlight),
   m_isCwndLimited(other.m_isCwndLimited),
   m_srtt(other.m_srtt),
   m_lastRtt(other.m_lastRtt),
   m_ecnMode(other.m_ecnMode),
   m_useEcn(other.m_useEcn),
   m_ectCodePoint(other.m_ectCodePoint),
   m_lastAckedSackedBytes(other.m_lastAckedSackedBytes),

   ////modSwift

   m_t_last_decrease(other.m_t_last_decrease),
   m_retransmit_count(other.m_retransmit_count),
   m_min_cwnd(other.m_min_cwnd),
   m_max_cwnd(other.m_max_cwnd),
   m_max_mdf(other.m_max_mdf) 

{
}

const char* const TcpSocketState::TcpCongStateName[TcpSocketState::CA_LAST_STATE] = {
 "CA_OPEN",
 "CA_DISORDER",
 "CA_CWR",
 "CA_RECOVERY",
 "CA_LOSS",
};

const char* const TcpSocketState::EcnStateName[TcpSocketState::ECN_CWR_SENT + 1] = {
 "ECN_DISABLED",
 "ECN_IDLE",
 "ECN_CE_RCVD",
 "ECN_SENDING_ECE",
 "ECN_ECE_RCVD",
 "ECN_CWR_SENT",
};

} // namespace ns3






