#ifndef __FOODDELIVERY_DISPATCHERAPP_H
#define __FOODDELIVERY_DISPATCHERAPP_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace fooddelivery {

using namespace inet;

//
// Central dispatcher. Forwards orders to a restaurant (round-robin) and, once
// the food is ready, forwards a delivery request to a rider (round-robin).
// Carries the customer address inside the message, so it keeps no per-order state.
//
class DispatcherApp : public cSimpleModule, public UdpSocket::ICallback
{
  protected:
    int localPort = -1;
    int destPort = -1;

    UdpSocket socket;
    std::vector<L3Address> restaurants;
    std::vector<L3Address> riders;
    int nextRestaurant = 0;
    int nextRider = 0;

    int receivedCount = 0;
    int dispatchedCount = 0;

    simsignal_t orderReceivedSignal;
    simsignal_t orderDispatchedSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    std::vector<L3Address> resolveList(const char *addresses);
    void forward(int orderId, int type, const char *customerAddress,
                 simtime_t orderTime, const L3Address &dest, const char *packetName);

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
};

} // namespace fooddelivery

#endif
