#include "RestaurantApp.h"
#include "OrderMessage_m.h"

#include "inet/common/InitStages.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace fooddelivery {

Define_Module(RestaurantApp);

RestaurantApp::~RestaurantApp()
{
    for (auto &entry : pending)
        cancelAndDelete(entry.second.timer);
    pending.clear();
}

void RestaurantApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        destPort = par("destPort");
        foodPreparedSignal = registerSignal("foodPrepared");
        prepDurationSignal = registerSignal("prepDuration");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);
    }
}

void RestaurantApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // Prep finished for one order.
        auto it = pending.find(msg->getId());
        if (it != pending.end()) {
            PendingOrder &po = it->second;

            auto chunk = makeShared<OrderChunk>();
            chunk->setOrderId(po.orderId);
            chunk->setOrderType(FOOD_READY);
            chunk->setCustomerAddress(po.customerAddress.c_str());
            chunk->setOrderTime(po.orderTime);
            chunk->setChunkLength(B(32));

            auto packet = new Packet("FoodReady");
            packet->insertAtBack(chunk);
            socket.sendTo(packet, po.dispatcherAddr, destPort);

            preparedCount++;
            emit(foodPreparedSignal, (long)po.orderId);
            emit(prepDurationSignal, (simTime() - po.requestTime).dbl());
            EV_INFO << "Restaurant finished order #" << po.orderId
                    << ", notifying dispatcher " << po.dispatcherAddr << endl;

            pending.erase(it);
        }
        delete msg;
    }
    else {
        socket.processMessage(msg);
    }
}

void RestaurantApp::socketDataArrived(UdpSocket *, Packet *packet)
{
    auto chunk = packet->peekAtFront<OrderChunk>();

    if (chunk->getOrderType() == PREP_REQUEST) {
        PendingOrder po;
        po.orderId = chunk->getOrderId();
        po.customerAddress = chunk->getCustomerAddress();
        po.orderTime = chunk->getOrderTime();
        po.dispatcherAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
        po.requestTime = simTime();

        cMessage *timer = new cMessage("prepDone");
        po.timer = timer;
        pending[timer->getId()] = po;

        simtime_t prep = par("prepTime");
        EV_INFO << "Restaurant received order #" << po.orderId
                << ", prep time " << prep << endl;
        scheduleAt(simTime() + prep, timer);
    }

    delete packet;
}

void RestaurantApp::socketErrorArrived(UdpSocket *, Indication *indication)
{
    EV_WARN << "Restaurant UDP error: " << indication->getName() << endl;
    delete indication;
}

void RestaurantApp::socketClosed(UdpSocket *)
{
}

void RestaurantApp::finish()
{
    recordScalar("ordersPrepared", preparedCount);
}

} // namespace fooddelivery
