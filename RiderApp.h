#ifndef __FOODDELIVERY_RIDERAPP_H
#define __FOODDELIVERY_RIDERAPP_H

#include <map>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace fooddelivery {

using namespace inet;

//
// Delivery rider. On a delivery request it waits a (random) deliveryTime, then
// sends a "delivered" message straight to the customer over the direct link.
//
class RiderApp : public cSimpleModule, public UdpSocket::ICallback
{
  protected:
    struct PendingDelivery {
        int orderId = 0;
        simtime_t orderTime;
        simtime_t requestTime;
        cMessage *timer = nullptr;
    };

    int localPort = -1;
    int destPort = -1;

    UdpSocket socket;
    L3Address customerAddr;                  // resolved from the customerAddress parameter
    std::map<long, PendingDelivery> pending; // keyed by timer id

    int deliveredCount = 0;

    simsignal_t orderDeliveredSignal;
    simsignal_t deliveryDurationSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    virtual ~RiderApp();
};

} // namespace fooddelivery

#endif
