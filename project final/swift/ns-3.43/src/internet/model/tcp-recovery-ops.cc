/*
 * Copyright (c) 2018 NITK Surathkal
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Viyom Mittal <viyommittal@gmail.com>
 *         Vivek Jain <jain.vivek.anand@gmail.com>
 *         Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *
 */
#include "tcp-recovery-ops.h"

#include "tcp-socket-state.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TcpRecoveryOps");

NS_OBJECT_ENSURE_REGISTERED(TcpRecoveryOps);

TypeId
TcpRecoveryOps::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TcpRecoveryOps").SetParent<Object>().SetGroupName("Internet");
    return tid;
}

TcpRecoveryOps::TcpRecoveryOps()
    : Object()
{
    NS_LOG_FUNCTION(this);
}

TcpRecoveryOps::TcpRecoveryOps(const TcpRecoveryOps& other)
    : Object(other)
{
    NS_LOG_FUNCTION(this);
}

TcpRecoveryOps::~TcpRecoveryOps()
{
    NS_LOG_FUNCTION(this);
}

void
TcpRecoveryOps::UpdateBytesSent(uint32_t bytesSent)
{
    NS_LOG_FUNCTION(this << bytesSent);
}

// Classic recovery

NS_OBJECT_ENSURE_REGISTERED(TcpClassicRecovery);

TypeId
TcpClassicRecovery::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TcpClassicRecovery")
                            .SetParent<TcpRecoveryOps>()
                            .SetGroupName("Internet")
                            .AddConstructor<TcpClassicRecovery>();
    return tid;
}

TcpClassicRecovery::TcpClassicRecovery()
    : TcpRecoveryOps()
{
    NS_LOG_FUNCTION(this);
}

TcpClassicRecovery::TcpClassicRecovery(const TcpClassicRecovery& sock)
    : TcpRecoveryOps(sock)
{
    NS_LOG_FUNCTION(this);
}

TcpClassicRecovery::~TcpClassicRecovery()
{
    NS_LOG_FUNCTION(this);
}

void
TcpClassicRecovery::EnterRecovery(Ptr<TcpSocketState> tcb,
                                  uint32_t dupAckCount,
                                  uint32_t unAckDataCount [[maybe_unused]],
                                  uint32_t deliveredBytes [[maybe_unused]])
{
    NS_LOG_FUNCTION(this << tcb << dupAckCount << unAckDataCount);
    
    // if you want to run any other tcp you need to change the 0->1 and where swift change the 1->0

    if(0){
        tcb->m_cWnd = tcb->m_ssThresh;
        tcb->m_cWndInfl = tcb->m_ssThresh + (dupAckCount * tcb->m_segmentSize);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////swift modification////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //swiftMod
    if(1)
    {
        uint32_t cwnd_prev = tcb->m_cWnd;
        bool can_decrease = tcb->CanDecrease();

        tcb->m_retransmit_count = 0;

        if (can_decrease)
        {
            tcb->m_cWnd = (1 - tcb->m_max_mdf)*tcb->m_cWnd;
        }


        tcb->Clamp();

        if (tcb->m_cWnd <= cwnd_prev)
        {
            tcb->m_t_last_decrease = Simulator::Now();
        }

        tcb->m_cWndInfl = tcb->m_cWnd + (dupAckCount * tcb->m_segmentSize);
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
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
TcpClassicRecovery::DoRecovery(Ptr<TcpSocketState> tcb,
                               uint32_t deliveredBytes [[maybe_unused]],
                               bool isDupAck)
{
    NS_LOG_FUNCTION(this << tcb << deliveredBytes << isDupAck);
    tcb->m_cWndInfl += tcb->m_segmentSize;
}

void
TcpClassicRecovery::ExitRecovery(Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this << tcb);
    // Follow NewReno procedures to exit FR if SACK is disabled
    // (RFC2582 sec.3 bullet #5 paragraph 2, option 2)
    // In this implementation, actual m_cWnd value is reset to ssThresh
    // immediately before calling ExitRecovery(), so we just need to
    // reset the inflated cWnd trace variable
    tcb->m_cWndInfl = tcb->m_ssThresh.Get();
}

std::string
TcpClassicRecovery::GetName() const
{
    return "TcpClassicRecovery";
}

Ptr<TcpRecoveryOps>
TcpClassicRecovery::Fork()
{
    return CopyObject<TcpClassicRecovery>(this);
}

} // namespace ns3
