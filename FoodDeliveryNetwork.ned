//
// Food Delivery Network topology -- DIRECT point-to-point links.
//
// Every pair of nodes that exchanges messages has its own dedicated Ethernet
// link (its own subnet), so each packet travels ONLY along its intended link.
// There is no switch, so there is no MAC-learning flooding: the live animation
// shows exactly the five message arrows of the workflow.
//
//   customer  ---L1---  dispatcher  ---L2---  restaurant
//      |                    |
//      L4                   L3
//      |                    |
//    rider  ----------------+
//
//   L1 customer  <-> dispatcher   (ORDER_REQUEST)
//   L2 dispatcher<-> restaurant   (PREP_REQUEST / FOOD_READY)
//   L3 dispatcher<-> rider        (DELIVERY_REQUEST)
//   L4 rider     <-> customer     (DELIVERED)
//
package fooddelivery;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth100M;

network FoodDeliveryNetwork
{
    parameters:
        @display("bgb=720,470");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=360,40;i=block/cogwheel");
        }
        customer: StandardHost {
            @display("p=140,150;i=device/laptop");
        }
        dispatcher: StandardHost {
            @display("p=380,250;i=device/server");
        }
        restaurant: StandardHost {
            @display("p=620,150;i=device/server2");
        }
        rider: StandardHost {
            @display("p=380,430;i=device/cellphone");
        }
    connections:
        customer.ethg++   <--> Eth100M <--> dispatcher.ethg++;   // L1
        dispatcher.ethg++ <--> Eth100M <--> restaurant.ethg++;   // L2
        dispatcher.ethg++ <--> Eth100M <--> rider.ethg++;        // L3
        rider.ethg++      <--> Eth100M <--> customer.ethg++;     // L4
}
