#include "tcp-swift.h"
#include "tcp-socket-state.h"
#include "ns3/simulator.h"
#include "rtt-estimator.h"
#include "ns3/nstime.h"
#include "tcp-socket-base.h"
#include "ns3/log.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpSwift");
NS_OBJECT_ENSURE_REGISTERED (TcpSwift);


TypeId TcpSwift::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TcpSwift")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpSwift> ()
    .SetGroupName ("Internet")
    .AddAttribute ("Alpha", "Lower bound of packets in network",
                   UintegerValue (2),
                   MakeUintegerAccessor (&TcpSwift::m_alpha),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Beta", "Upper bound of packets in network",
                   UintegerValue (4),
                   MakeUintegerAccessor (&TcpSwift::m_beta),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Mode", "mode for target delay",
                    UintegerValue (1),
                    MakeUintegerAccessor (&TcpSwift::m_mode),
                    MakeUintegerChecker<uint32_t> ());
  return tid;
}

TcpSwift::TcpSwift ()
  : TcpNewReno (),
    m_baseRtt (Time::Max ())
{
  NS_LOG_FUNCTION (this);
 // std::cout << "swift running" << std::endl;
}

TcpSwift::~TcpSwift ()
{
  NS_LOG_FUNCTION (this);
}

TcpSwift::TcpSwift (const TcpSwift& sock)
  : TcpNewReno (sock),
    m_alpha (sock.m_alpha),
    m_beta (sock.m_beta),
    m_baseRtt (sock.m_baseRtt)
{
  NS_LOG_FUNCTION (this);
//  std::cout << "swift running - from copy constructor" << std::endl;
}


Ptr<TcpCongestionOps> TcpSwift::Fork ()
{
  return CopyObject<TcpSwift> (this);
}

std::string TcpSwift::GetName () const
{
  return "TcpSwift";
}

void TcpSwift::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time& rtt){
    tcb->m_retransmit_count = 0;    /// so retransmit wont enter panic mode ///// we might need to change this line and put it in increase window, the thing is do we need when recieving any ack we need to update this paramater to zero? or only when we recieve a new ack that is not duplicated so basically need to be xche


    // 1. Update the base RTT (minimum observed RTT)
    if (!rtt.IsZero() && (rtt < m_baseRtt))
    {
        m_baseRtt = rtt; // Update base RTT
        NS_LOG_DEBUG ("Updated m_baseRtt = " << m_baseRtt);
    }else{

        NS_LOG_DEBUG ("Updated m_baseRtt = " << m_baseRtt);
    }
}

void TcpSwift::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked){    //amir 5

    uint32_t cwnd_prev = tcb->m_cWnd;
    bool can_decrease = tcb->CanDecrease();

    // if you want to use slowstart with swift change the 0->1
    if((tcb->m_cWnd < tcb->m_ssThresh) && 0)
    {    
        // Slow start mode
        // NS_LOG_LOGIC ("We are in slow start and diff < m_gamma, so we "
        // "follow NewReno slow start");
        //std::cout << "We are in slow start and diff < m_gamma, so we follow NewReno slow start" << std::endl;
            
        TcpNewReno::SlowStart (tcb, segmentsAcked);
        //NS_LOG_LOGIC ("Cond0=> cwnd = "+std::to_string(tcb->m_cWnd)+"\n");
        NS_LOG_LOGIC("Cond0=> cwnd = " << std::to_string(tcb->m_cWnd) << ", " << std::to_string(tcb->m_ssThresh) << "\n");

    }
    else
    {     
        double segCwnd = (double)(tcb->m_cWnd)/(double)(tcb->m_segmentSize);
        double target_delay = TargetDelay();

        double segCwndupdate = (double)(tcb->m_cWnd)/(double)(tcb->m_segmentSize);

        if (tcb->m_lastRtt.Get().GetSeconds() < target_delay)    /// check casting
        {
            if( segCwnd >= 1)
            {
              
                segCwndupdate = segCwndupdate + ((double)m_alpha/(double)segCwndupdate)*segmentsAcked;   /// we are assuming that segmentsAcked is the num_acked;

                tcb->m_cWnd = segCwndupdate * tcb->m_segmentSize; 
            }
            else
            {
                segCwndupdate = segCwndupdate + m_alpha*segmentsAcked;     /// we are assuming that segmentsAcked is the num_acked;

                tcb->m_cWnd = segCwndupdate * tcb->m_segmentSize; 
            }
        }
        else
        {
            if(can_decrease)
            {   
                double value1 = 1 - (m_beta*(tcb->m_lastRtt.Get().GetSeconds()-target_delay)/tcb->m_lastRtt.Get().GetSeconds());
                double value2 = (1-tcb->m_max_mdf);
                tcb->m_cWnd = std::max(value1,value2) * tcb->m_cWnd;
            }
        }


      tcb->Clamp();

      if (tcb->m_cWnd <= cwnd_prev)
      {
          tcb->m_t_last_decrease = Simulator::Now();
      }

      tcb->m_cWndInfl = tcb->m_cWnd;

      if (tcb->m_cWnd < tcb->m_segmentSize)
      {
       
        double temp_rtt = tcb->m_lastRtt.Get().GetSeconds();
        if (temp_rtt < 0.001)
        {
            temp_rtt = 0.001;
        }
        
        tcb->m_pacingRate = (tcb->m_cWnd * 8)/ temp_rtt;

        
      }else{
        tcb->m_pacingRate = tcb->m_maxPacingRate;
      }
      NS_LOG_DEBUG ("after pacing  = " << tcb->m_cWnd);
    }
}






  double TcpSwift::TargetDelay()
  {
      double target_delay;

      if (m_mode == 0)
      {
        target_delay = m_baseRtt.GetSeconds();
      }
      else if (m_mode == 1)
      {
        target_delay = m_baseRtt.GetSeconds() + 0.001;
      }
      else
      {
        target_delay = 0.025;
      }
      
      return target_delay;
  }

}





