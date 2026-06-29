# Food Delivery Tracking Network — OMNeT++ / INET simulation

A small, self-contained INET simulation that models the network communication of a
food-delivery service. Four hosts — a **customer**, a **dispatcher**, a
**restaurant** and a **delivery rider** — are connected by direct point-to-point
Ethernet links, and a single order travels between them as UDP packets through five
stages, from placement to delivery confirmation.

Computer Networks Lab (CSE-3634), IIUC — Group E.

## Order workflow

```
customer ──ORDER_REQUEST──▶ dispatcher ──PREP_REQUEST──▶ restaurant
                                                            │ (prep time)
customer ◀──DELIVERED── rider ◀──DELIVERY_REQUEST── dispatcher ◀──FOOD_READY──┘
```

The customer measures the end-to-end delay (order placed → "delivered" received).
The dispatcher is stateless and assigns work in round-robin order.

## Files

| File | Purpose |
|------|---------|
| `package.ned` | Package + C++ namespace declaration |
| `FoodDeliveryNetwork.ned` | Topology: 4 hosts on direct point-to-point links (no switch) |
| `Apps.ned` | NED declarations of the four applications |
| `OrderMessage.msg` | `OrderChunk` packet + `OrderType` enum |
| `config.xml` | Explicit IP addresses (one subnet per link) |
| `CustomerApp.{h,cc}` | Generates orders, records end-to-end delay |
| `DispatcherApp.{h,cc}` | Routes orders to restaurant, then rider |
| `RestaurantApp.{h,cc}` | Food preparation timing |
| `RiderApp.{h,cc}` | Delivery timing and confirmation |
| `omnetpp.ini` | Two run configs: `General` and `HighLoad` |

## Requirements

- OMNeT++ 6.x
- INET Framework 4.x

## Build & run

1. Import the project into the OMNeT++ IDE and make sure it references **INET**.
2. Build the project (Ctrl+B). The `.msg` file is compiled automatically.
3. Run `omnetpp.ini` and choose a configuration:
   - `General` — an order roughly every 5 s (light load)
   - `HighLoad` — an order roughly every 1.5 s (busy load)

Command line:

```bash
opp_makemake -f --deep -I$INET_ROOT/src -L$INET_ROOT/src -lINET
make
./FoodDelivery -u Cmdenv -c General
./FoodDelivery -u Cmdenv -c HighLoad
```

## Statistics collected

End-to-end order delay, preparation and delivery durations, and per-node order
counts (placed / received / dispatched / prepared / delivered). Open the resulting
`.sca` / `.vec` files in the IDE's Analysis tool to plot delay and throughput.

## Sample results (300 s run)

| Metric | General | HighLoad |
|--------|---------|----------|
| Orders placed → delivered | 56 → 54 | 193 → 187 |
| Mean end-to-end delay | 13.47 s | 13.59 s |
| Throughput | 0.18 orders/s | 0.62 orders/s |

The mean delay stays ~13.5 s under both loads (≈ 3.5 s prep + 10 s delivery)
because the single restaurant and rider handle concurrent orders in parallel.
