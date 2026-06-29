#ifndef __FOODDELIVERY_RESTAURANTAPP_H
#define __FOODDELIVERY_RESTAURANTAPP_H

#include <map>
#include <string>

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace fooddelivery {

using namespace inet;

//
// Restaurant. On a prep request it waits a (random) prepTime, then sends a
// "food ready" message back to the dispatcher.
//
class RestaurantApp : public cSimpleModule, public UdpSocket::ICallback
{
  protected:
    // Per-order information kept while the food is being prepared.
    struct PendingOrder {
        int orderId = 0;
        std::string customerAddress;
        simtime_t orderTime;
        L3Address dispatcherAddr;
        simtime_t requestTime;
        cMessage *timer = nullptr;
    };

    int localPort = -1;
    int destPort = -1;

    UdpSocket socket;
    std::map<long, PendingOrder> pending;   // keyed by timer id

    int preparedCount = 0;
    long nextTimerId = 0;

    simsignal_t foodPreparedSignal;
    simsignal_t prepDurationSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    virtual ~RestaurantApp();
};

} // namespace fooddelivery

#endif
