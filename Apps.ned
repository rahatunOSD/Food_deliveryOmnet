//
// Application modules for the Food Delivery Network.
// Every app implements the INET IApp interface (gates socketIn / socketOut).
//
package fooddelivery;

import inet.applications.contract.IApp;

//
// Generates food orders and sends them to the dispatcher over UDP.
// Records the end-to-end delay once a "delivered" reply comes back.
//
simple CustomerApp like IApp
{
    parameters:
        int localPort = default(5000);
        int destPort = default(5000);
        string dispatcherAddress;                            // e.g. "10.0.1.2"
        double startTime @unit(s) = default(1s);
        volatile double sendInterval @unit(s) = default(exponential(5s));
        int numOrders = default(-1);
        @display("i=block/app");
        @signal[endToEndDelay](type=double);
        @signal[orderCompleted](type=long);
        @statistic[endToEndDelay](title="order end-to-end delay"; unit=s; record=mean,max,min,vector; interpolationmode=none);
        @statistic[orderCompleted](title="orders completed"; record=count);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

//
// Central dispatcher. Forwards an order to a restaurant, then to a rider.
//
simple DispatcherApp like IApp
{
    parameters:
        int localPort = default(5000);
        int destPort = default(5000);
        string restaurantAddresses;   // e.g. "10.0.2.2"
        string riderAddresses;        // e.g. "10.0.3.2"
        @display("i=block/app");
        @signal[orderReceived](type=long);
        @signal[orderDispatched](type=long);
        @statistic[orderReceived](title="orders received"; record=count);
        @statistic[orderDispatched](title="orders dispatched to rider"; record=count);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

//
// Restaurant. On a prep request it waits prepTime, then replies "food ready".
//
simple RestaurantApp like IApp
{
    parameters:
        int localPort = default(5000);
        int destPort = default(5000);
        volatile double prepTime @unit(s) = default(uniform(2s, 5s));
        @display("i=block/app");
        @signal[foodPrepared](type=long);
        @signal[prepDuration](type=double);
        @statistic[foodPrepared](title="orders prepared"; record=count);
        @statistic[prepDuration](title="prep duration"; unit=s; record=mean,max,vector; interpolationmode=none);
    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

//
// Delivery rider. On a delivery request it waits deliveryTime, then sends a
// "delivered" message directly to the customer (customerAddress).
//
simple RiderApp like IApp
{
    parameters:
        int localPort = default(5000);
        int destPort = default(5000);
        string customerAddress;       // e.g. "10.0.4.2" (direct rider->customer link)
        volatile double deliveryTime @unit(s) = default(uniform(5s, 15s));
        @display("i=block/app");
        @signal[orderDelivered](type=long);
        @signal[deliveryDuration](type=double);
        @statistic[orderDelivered](title="orders delivered"; record=count);
        @statistic[deliveryDuration](title="delivery duration"; unit=s; record=mean,max,vector; interpolationmode=none);
    gates:
        input so