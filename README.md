# Food Delivery Network — OMNeT++ / INET simulation

A small, self-contained INET simulation that models a food-delivery service as a
real UDP/IP network. Four hosts on one switched LAN play the roles of a
**customer**, a **dispatcher**, a **restaurant**, and a **delivery rider**, and a
food order travels end-to-end as actual UDP packets.

## The workflow

```
  customer            dispatcher          restaurant            rider
     |  ORDER_REQUEST     |                    |                  |
     | -----------------> |  PREP_REQUEST      |                  |
     |                    | -----------------> | (prepTime)       |
     |                    |   FOOD_READY       |                  |
     |                    | <----------------- |                  |
     |                    |  DELIVERY_REQUEST                     |
     |                    | -----------------------------------> | (deliveryTime)
     |       DELIVERED                                            |
     | <-------------------------------------------------------- |
```

The customer measures the **end-to-end delay** (order placed → "delivered"
received). The dispatcher is stateless: the customer's address rides inside the
message, so any restaurant/rider can be added without extra bookkeeping.

## Files

| File | Purpose |
|------|---------|
| `package.ned` | Declares the `fooddelivery` package and maps it to the C++ namespace. |
| `FoodDeliveryNetwork.ned` | Network topology (4 StandardHosts + EtherSwitch + IPv4 configurator). |
| `Apps.ned` | NED declarations of the four apps (all implement INET's `IApp`). |
| `OrderMessage.msg` | `OrderChunk` packet definition + `OrderType` enum. |
| `CustomerApp.{h,cc}` | Generates orders, records end-to-end delay. |
| `DispatcherApp.{h,cc}` | Routes orders to a restaurant, then to a rider (round-robin). |
| `RestaurantApp.{h,cc}` | Waits `prepTime`, then replies "food ready". |
| `RiderApp.{h,cc}` | Waits `deliveryTime`, then delivers to the customer. |
| `omnetpp.ini` | Two run configs: `General` and `HighLoad`. |

> All files live in one flat folder, which is fine for OMNeT++. If you prefer,
> move the `.h/.cc/.msg` into a `src/` subfolder — no code changes are needed.

## Requirements

- **OMNeT++ 6.x** (also builds on 5.7)
- **INET 4.x** (4.4 / 4.5 recommended) — the apps use the INET 4.x packet/chunk
  API (`Packet`, `makeShared`, `UdpSocket::ICallback`).

## Setup in the OMNeT++ IDE

1. Make sure the **INET** project is already imported and built in your workspace.
2. `File → New → OMNeT++ Project`, name it e.g. `fooddelivery`. Copy all the files
   from this folder into the new project (or `File → Import → General → File System`).
3. Add the INET dependency so headers and the library are found:
   - Right-click the project → **Properties → Project References** → tick **inet**.
   - Properties → **OMNeT++ → Makemake** → on the source folder choose
     *Build → "Export this folder's compiler/linker options"* is not needed; the
     project reference handles include paths and linking automatically when INET
     is referenced.
4. Build the project (Ctrl+B). The message compiler turns `OrderMessage.msg`
   into `OrderChunk_m.h/.cc` automatically during the build.

### Command-line build (optional)

```bash
# from this folder, with $INET_ROOT pointing at your INET install
opp_makemake -f --deep -I$INET_ROOT/src -L$INET_ROOT/src -lINET
make
```

## Running

- In the IDE: right-click `omnetpp.ini` → **Run As → OMNeT++ Simulation**.
- Pick the config when prompted:
  - **General** — orders every ~5 s on average.
  - **HighLoad** — orders every ~1.5 s (queues and delays grow).
- Use **Qtenv** to watch packets flow between the hosts; use **Cmdenv** for fast
  batch runs.

Command line:

```bash
./fooddelivery -u Cmdenv -c General omnetpp.ini
./fooddelivery -u Cmdenv -c HighLoad omnetpp.ini
```

## Statistics collected

| Module | Statistic | Meaning |
|--------|-----------|---------|
| customer | `endToEndDelay` | order placed → delivered (mean/max/min/vector) |
| customer | `ordersPlaced`, `ordersDelivered` | scalars at end of run |
| dispatcher | `orderReceived`, `orderDispatched` | counts |
| restaurant | `foodPrepared`, `prepDuration` | count + prep time |
| rider | `orderDelivered`, `deliveryDuration` | count + delivery time |

Open the resulting `.sca` / `.vec` files in the IDE's **Analysis** tool to plot
delays and counts.

## Extending it

- **More restaurants/riders:** add hosts in `FoodDeliveryNetwork.ned`, connect
  them to the switch, then list them in the ini, e.g.
  `*.dispatcher.app[0].restaurantAddresses = "restaurant1 restaurant2"`.
  The dispatcher already round-robins across the list.
- **Different load:** change `sendInterval`, `prepTime`, `deliveryTime` in the ini.
- **Smarter routing:** replace the round-robin in `DispatcherApp::socketDataArrived`
  with a least-loaded or nearest-rider policy.
