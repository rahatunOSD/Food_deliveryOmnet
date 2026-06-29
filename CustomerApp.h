#ifndef __FOODDELIVERY_CUSTOMERAPP_H
#define __FOODDELIVERY_CUSTOMERAPP_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace fooddelivery {

using namespace inet;

//
// Generates food orders towards the dispatcher and measures the end-to-end
// delay (order created -> "delivered" received).
//
class CustomerApp : public cSimpleModule, public UdpSocket::ICallback
{
  protected:
    // parameters
    int localPort = -1;
    int destPort = -1;
    int numOrders = -1;
    L3Address dispatcherAddr;

    // state
    UdpSocket socket;
    cMessage *sendTimer = nullptr;
    int orderCounter = 0;
    int completedCount = 0;

    // signals
    simsignal_t endToEndDelaySignal;
    simsignal_t orderCompletedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    void sendOrder();

    // UdpSocket::ICallback
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    virtual ~CustomerApp();
};

} // namespace fooddelivery

#endif
