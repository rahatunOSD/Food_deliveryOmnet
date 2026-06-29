//
// Message definition for the Food Delivery Network.
//
// OrderChunk is an INET packet chunk (FieldsChunk) that is inserted into a
// UDP Packet. The same chunk type is reused for every hop of the workflow;
// the "orderType" field tells the receiver which stage this is.
//
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace fooddelivery;

//
// Stage of the order workflow.
//
enum OrderType
{
    ORDER_REQUEST    = 0;   // customer   -> dispatcher
    PREP_REQUEST     = 1;   // dispatcher -> restaurant
    FOOD_READY       = 2;   // restaurant -> dispatcher
    DELIVERY_REQUEST = 3;   // dispatcher -> rider
    DELIVERED        = 4;   // rider      -> customer
};

//
// Payload carried by every food-delivery packet.
//
class OrderChunk extends inet::FieldsChunk
{
    int orderId = 0;                  // unique id assigned by the customer
    int orderType = ORDER_REQUEST;    // current stage (see OrderType)
    string customerAddress;           // customer's L3 address (string form)
    simtime_t orderTime = 0;          // time the order was created (for end-to-end delay)
}
