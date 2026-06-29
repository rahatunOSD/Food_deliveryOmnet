#include "RiderApp.h"
#include "OrderMessage_m.h"

#include "inet/common/InitStages.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace fooddelivery {

Define_Module(RiderApp);

RiderApp::~RiderApp()
{
    for (auto &entry : pending)
        cancelAndDelete(entry.second.timer);
    pending.clear();
}

void RiderApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        destPort = par("destPort");
        orderDeliveredSignal = registerSignal("orderDelivered");
        deliveryDurationSignal = registerSignal("deliveryDuration");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);

        // Address of the customer, reached over the direct rider--customer link.
        customerAddr = L3AddressResolver().resolve(par("customerAddress"));
    }
}

void RiderApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        auto it = pending.find(msg->getId());
        if (it != pending.end()) {
            PendingDelivery &pd = it->second;

            auto chunk = makeShared<OrderChunk>();
            chunk->setOrderId(pd.orderId);
            chunk->setOrderType(DELIVERED);
            chunk->setOrderTime(pd.orderTime);
            chunk->setChunkLength(B(32));

            auto packet = new Packet("Delivered");
            packet->insertAtBack(chunk);
            socket.sendTo(packet, customerAddr, destPort);

            deliveredCount++;
            emit(orderDeliveredSignal, (long)pd.orderId);
            emit(deliveryDurationSignal, (simTime() - pd.requestTime).dbl());
            EV_INFO << "Rider delivered order #" << pd.orderId
                    << " to customer " << customerAddr << endl;

            pending.erase(it);
        }
        delete msg;
    }
    else {
        socket.processMessage(msg);
    }
}

void RiderApp::socketDataArrived(UdpSocket *, Packet *packet)
{
    auto chunk = packet->peekAtFront<OrderChunk>();

    if (chunk->getOrderType() == DELIVERY_REQUEST) {
        PendingDelivery pd;
        pd.orderId = chunk->getOrderId();
        pd.orderTime = chunk->getOrderTime();
        pd.requestTime = simTime();

        cMessage *timer = new cMessage("deliveryDone");
        pd.timer = timer;
        pending[timer->getId()] = pd;

        simtime_t delivery = par("deliveryTime");
        EV_INFO << "Rider picked up order #" << pd.orderId
                << ", delivery time " << delivery << endl;
        scheduleAt(simTime() + delivery, timer);
    }

    delete packet;
}

void RiderApp::socketErrorArrived(UdpSocket *, Indication *indication)
{
    EV_WARN << "Rider UDP error: " << indication->getName() << endl;
    delete indication;
}

void RiderApp::socketClosed(UdpSocket *)
{
}

void RiderApp::finish()
{
    recordScalar("ordersDelivered", deliveredCount);
}

} // namespace fooddelivery
