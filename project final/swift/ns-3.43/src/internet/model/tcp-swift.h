#ifndef TCPSWIFT_H
#define TCPSWIFT_H

#include "tcp-congestion-ops.h"
#include "tcp-socket-base.h"

namespace ns3 {

class TcpSocketState;

class TcpSwift : public TcpNewReno
{ 
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Create an unbound tcp socket.
   */
  TcpSwift ();

  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpSwift (const TcpSwift& sock);
  virtual ~TcpSwift ();

  virtual std::string GetName () const;

  /**
   * \brief Compute RTTs needed to execute Swift algorithm
   *
   * The function filters RTT samples from the last RTT to find
   * the current smallest propagation delay + queueing delay (minRtt).
   * We take the minimum to avoid the effects of delayed ACKs.
   *
   * The function also min-filters all RTT measurements seen to find the
   * propagation delay (baseRtt).
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   * \param rtt last RTT
   *
   */
  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                          const Time& rtt);

  /**
   * \brief Enable/disable Swift algorithm depending on the congestion state
   *
   * We only start a Swift cycle when we are in normal congestion state (CA_OPEN state).
   *
   * \param tcb internal congestion state
   * \param newState new congestion state to which the TCP is going to switch
   */
  //virtual void CongestionStateSet (Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState);

  /**
   * \brief Adjust cwnd following Swift linear increase/decrease algorithm
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   */
  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  /**
   * \brief Get slow start threshold following Swift principle
   *
   * \param tcb internal congestion state
   * \param bytesInFlight bytes in flight
   *
   * \return the slow start threshold value
   */
 // virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight);

  // virtual Ptr<TcpSocketBase> Fork ();
  virtual Ptr<TcpCongestionOps> Fork ();  //amir
  // virtual void ReTxTimeout ();
  
  //expirement
 // uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight);

 // virtual bool HasCongControl()const override;

protected:
private:
  /**
   * \brief Enable Swift algorithm to start taking Swift samples
   *
   * Swift algorithm is enabled in the following situations:
   * 1. at the establishment of a connection
   * 2. after an RTO
   * 3. after fast recovery
   * 4. when an idle connection is restarted
   *
   * \param tcb internal congestion state
   */
  double TargetDelay ();


private:
  uint32_t m_alpha;                  //!< Alpha threshold, lower bound of packets in network
  uint32_t m_beta;                   //!< Beta threshold, upper bound of packets in network
  Time m_baseRtt;                    //!< Minimum of all Swift RTT measurements seen during connection  
  uint32_t m_mode;
};

} // namespace ns3

#endif // TCPSWIFT_H