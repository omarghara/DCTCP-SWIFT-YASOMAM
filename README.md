# Fat-Tree Topology Simulation with DCTCP and Swift  

This repository provides a **Fat-Tree topology** implementation for network simulations, supporting configurable numbers of nodes. It includes:  

- A **TCP application** that runs on the Fat-Tree topology.  
- A **DCTCP (Data Center TCP)** application designed to demonstrate data center traffic.  
- Optional integration with **NetAnim** for animation and visualization.  

---

## Features  
- **Fat-Tree Topology**: Simulate data center networks with customizable node counts.  
- **DCTCP Application**: Evaluate performance using Data Center TCP over the Fat-Tree.  
- **NetAnim Support**: Visualize simulations using NetAnim (optional).  

---

## How to Use  

### Prerequisites  
1. Install **NS-3** following the [official guide](https://www.nsnam.org/wiki/Installation).  
2. (Optional) Install **NetAnim** for animation support.  

### Running Simulations  
1. Place the desired simulation file (e.g., DCTCP application) in the **`scratch`** directory of your NS-3 installation.  
2. Run the simulation using the command:  
   ```bash  
   ./ns3 run scratch/<filename>  
   ```  
3. If you haven't installed NetAnim, ensure all **NetAnim-related sections** and `#include` directives are commented out in the simulation file.

### Enabling NetAnim  
To enable NetAnim:  
1. Install **NetAnim** from the [official site](https://www.nsnam.org/wiki/NetAnim).  
2. Uncomment the **NetAnim section** in the simulation file you wish to run.  

---

## Current Limitations  
- **Swift Transport Protocol**: We are actively working on implementing the Swift protocol. Once complete, the implementation will be added to this repository.  

---

## Contributing  
Contributions are welcome! If you have ideas, bug reports, or feature requests, feel free to open an issue or submit a pull request.  
