#include "DispatcherApp.h"
#include "OrderMessage_m.h"

#include "inet/common/InitStages.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace fooddelivery {

Define_Module(DispatcherApp);

void DispatcherApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        destPort = par("destPort");
        orderReceivedSignal = registerSignal("orderReceived");
        orderDispatchedSignal = registerSignal("orderDispatched");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);

        restaurants = resolveList(par("restaurantAddresses"));
        riders = resolveList(par("riderAddresses"));

        if (restaurants.empty())
            throw cRuntimeError("DispatcherApp: no restaurant addresses configured");
        if (riders.empty())
            throw cRuntimeError("DispatcherApp: no rider addresses configured");
    }
}

std::vector<L3Address> DispatcherApp::resolveList(const char *addresses)
{
    std::vector<L3Address> result;
    cStringTokenizer tokenizer(addresses);
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr)
        result.push_back(L3AddressResolver().resolve(token));
    return result;
}

void DispatcherApp::handleMessage(cMessage *msg)
{
    // Dispatcher is purely reactive; everything arrives via the UDP socket.
    socket.processMessage(msg);
}

void DispatcherApp::forward(int orderId, int type, const char *customerAddress,
        simtime_t orderTime, const L3Address &dest, const char *packetName)
{
    auto chunk = makeShared<OrderChunk>();
    chunk->setOrderId(orderId);
    chunk->setOrderType(type);
    chunk->setCustomerAddress(customerAddress);
    chunk->setOrderTime(orderTime);
    chunk->setChunkLength(B(32));

    auto packet = new Packet(packetName);
    packet->insertAtBack(chunk);
    socket.sendTo(packet, dest, destPort);
}

void DispatcherApp::socketDataArrived(UdpSocket *, Packet *packet)
{
    auto chunk = packet->peekAtFront<OrderChunk>();
    int type = chunk->getOrderType();
    int orderId = chunk->getOrderId();
    simtime_t orderTime = chunk->getOrderTime();

    if (type == ORDER_REQUEST) {
        // Learn the customer's address from the packet's source tag.
        L3Address customerAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
        std::string customerStr = customerAddr.str();

        L3Address restaurant = restaurants[nextRestaurant];
        nextRestaurant = (nextRestaurant + 1) % restaurants.size();

        receivedCount++;
        emit(orderReceivedSignal, (long)orderId);
        EV_INFO << "Dispatcher received order #" << orderId << " from " << customerStr
                << ", forwarding to restaurant " << restaurant << endl;

        forward(orderId, PREP_REQUEST, customerStr.c_str(), orderTime, restaurant, "PrepRequest");
    }
    else if (type == FOOD_READY) {
        const char *customerStr = chunk->getCustomerAddress();

        L3Address rider = riders[nextRider];
        nextRider = (nextRider + 1) % riders.size();

        dispatchedCount++;
        emit(orderDispatchedSignal, (long)orderId);
        EV_INFO << "Dispatcher: order #" << orderId << " ready, assigning rider " << rider << endl;

        forward(orderId, DELIVERY_REQUEST, customerStr, orderTime, rider, "DeliveryRequest");
    }

    delete packet;
}

void DispatcherApp::socketErrorArrived(UdpSocket *, Indication *indication)
{
    EV_WARN << "Dispatcher UDP error: " << indication->getName() << endl;
    delete indication;
}

void DispatcherApp::socketClosed(UdpSocket *)
{
}

void DispatcherApp::finish()
{
    recordScalar("ordersReceived", receivedCount);
    recordScalar("ordersDispatched", dispatchedCount);
}

} // namespace fooddelivery
