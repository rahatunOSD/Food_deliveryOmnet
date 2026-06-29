#include "CustomerApp.h"
#include "OrderMessage_m.h"

#include "inet/common/InitStages.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace fooddelivery {

Define_Module(CustomerApp);

CustomerApp::~CustomerApp()
{
    cancelAndDelete(sendTimer);
}

void CustomerApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        destPort = par("destPort");
        numOrders = par("numOrders");
        endToEndDelaySignal = registerSignal("endToEndDelay");
        orderCompletedSignal = registerSignal("orderCompleted");
        sendTimer = new cMessage("sendOrder");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);

        // Resolve the dispatcher address (module path -> L3 address).
        dispatcherAddr = L3AddressResolver().resolve(par("dispatcherAddress"));

        simtime_t start = par("startTime");
        scheduleAt(simTime() + start, sendTimer);
    }
}

void CustomerApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        sendOrder();
    }
    else {
        socket.processMessage(msg);
    }
}

void CustomerApp::sendOrder()
{
    int id = ++orderCounter;

    auto chunk = makeShared<OrderChunk>();
    chunk->setOrderId(id);
    chunk->setOrderType(ORDER_REQUEST);
    chunk->setOrderTime(simTime());
    chunk->setChunkLength(B(32));

    auto packet = new Packet("OrderRequest");
    packet->insertAtBack(chunk);

    EV_INFO << "Customer placing order #" << id << " to dispatcher " << dispatcherAddr << endl;
    socket.sendTo(packet, dispatcherAddr, destPort);

    // Schedule the next order unless we have reached the configured limit.
    if (numOrders < 0 || orderCounter < numOrders) {
        simtime_t interval = par("sendInterval");
        scheduleAt(simTime() + interval, sendTimer);
    }
}

void CustomerApp::socketDataArrived(UdpSocket *, Packet *packet)
{
    auto chunk = packet->peekAtFront<OrderChunk>();

    if (chunk->getOrderType() == DELIVERED) {
        simtime_t delay = simTime() - chunk->getOrderTime();
        completedCount++;
        emit(endToEndDelaySignal, delay.dbl());
        emit(orderCompletedSignal, (long)chunk->getOrderId());
        EV_INFO << "Customer received order #" << chunk->getOrderId()
                << " delivered after " << delay << endl;
    }

    delete packet;
}

void CustomerApp::socketErrorArrived(UdpSocket *, Indication *indication)
{
    EV_WARN << "Customer UDP error: " << indication->getName() << endl;
    delete indication;
}

void CustomerApp::socketClosed(UdpSocket *)
{
}

void CustomerApp::finish()
{
    EV_INFO << "Customer placed " << orderCounter << " orders, "
            << completedCount << " delivered." << endl;
    recordScalar("ordersPlaced", orderCounter);
    recordScalar("ordersDelivered", completedCount);
}

} // namespace fooddelivery
