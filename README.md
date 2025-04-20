
# 🚀 NS-3 Swift & DCTCP Data Center Simulation

This project simulates data center congestion control protocols using NS-3. It includes a full implementation of **Swift** and utilizes **DCTCP** from standard NS-3. You can run both protocols, compare them under various topologies, and visualize the results with NetAnim.

---

## ✅ Compatibility

> **This setup is fully compatible with NS-3 version `3.43`.**  
> All modifications, configurations, and file changes are built and tested specifically for this version.

---

## 🧠 Key Highlights

- ✅ Swift congestion control with delay-based feedback (baseRTT, baseRTT+ε, or static delay)
- ✅ Uses DCTCP as implemented in NS-3
- ⚠️ To avoid protocol conflicts, **maintain two separate NS-3 installations**:
  - One for **DCTCP** and standard TCP
  - One for **Swift**, which modifies core TCP files
- 🧪 Includes simulation automation via `run_swift.sh`
- 🖥️ NetAnim visualization with a **simplified fat-tree** (`tcp-fat-tree-netanim`)

---

## 📦 Setup

### A1. Download and Build NS-3 (v3.43)
```bash
wget https://www.nsnam.org/release/ns-allinone-3.43.tar.bz2
tar -xvf ns-allinone-3.43.tar.bz2
cd ns-allinone-3.43
./build.py --enable-examples --enable-tests
./test.py
```

---

## 🔁 A2. Integrate Swift/DCTCP Simulator

```bash
git clone https://github.com/omarghara/DCTCP-SWIFT-YASOMAM
cd DCTCP-SWIFT-YASOMAM
```

Copy the following files into your `ns-3.43` directory:

| Target Directory         | Action    | Files                                                                 |
|--------------------------|-----------|------------------------------------------------------------------------|
| `src/internet/model/`    | ✅ Add     | `tcp-swift.cc`, `tcp-swift.h`                                         |
| `src/internet/model/`    | ✅ Replace | `tcp-congestion-ops.cc/h`, `tcp-recovery-ops.cc/h`, `tcp-socket-base.cc/h`, `tcp-socket-state.cc/h` |
| `src/internet/`          | ✅ Modify  | `CMakeLists.txt` → add Swift source files                             |
| `scratch/`               | ✅ Add     | `swift_cmd.cc`, `run_swift.sh`, `tcp-fat-tree-netanim.cc`             |

> 💡 Use `cp` to copy files into the correct directories.  
> 🛠 All changes are guaranteed to work with **NS-3 v3.43**.

---

## ⚙️ A3. Rebuild NS-3 with Swift

```bash
cd ns-3.43
./ns3 configure
./ns3 build
```

---

## ▶️ A4. Run a Simulation (Example)

```bash
./ns3 run "scratch/swift_cmd \
  --protocol=swift \
  --mode=2 \
  --k=4 \
  --p2p_DataRate=10Gbps \
  --p2p_Delay=10us \
  --num_of_flows=50 \
  --app_packet_size=1448 \
  --app_data_rate=100Mbps \
  --flowmonitor_start_time=0.1 \
  --simulation_stop_time=10 \
  --swift_mode=1"
```

Or batch run with:
```bash
./run_swift.sh
```

---

## 🧩 A5. Parameters Explained

| Parameter               | Description                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| `--protocol`           | Choose: `swift` or `dctcp`                                                  |
| `--mode`               | Flow behavior: <br> 0 = All to last host <br> 1 = All to one random host <br> 2 = Fully random |
| `--k`                  | Fat-tree switch port count                                                  |
| `--p2p_DataRate`       | Link bandwidth (e.g., `10Gbps`)                                             |
| `--p2p_Delay`          | Link delay (e.g., `10us`)                                                   |
| `--num_of_flows`       | Number of concurrent flows                                                  |
| `--app_packet_size`    | Application packet size in bytes                                            |
| `--app_data_rate`      | Flow-level app traffic rate (e.g., `100Mbps`)                               |
| `--swift_mode`         | Swift delay target logic: <br> 0 = baseRTT <br> 1 = baseRTT+ε <br> 2 = static fixed delay |
| `--simulation_stop_time` | Simulation duration                                                       |

---

## 📊 A6. Visualizing Results with NetAnim

We provide a NetAnim-compatible fat-tree version:  
➡️ **`tcp-fat-tree-netanim.cc`** in `scratch/`  

### 🛠️ Build & Run NetAnim
```bash
cd ../netanim
qmake NetAnim.pro
make
./NetAnim
```

### 📂 Open Simulation Output
```bash
./NetAnim ../ns-3.43/scratch/swift-animation.xml
```

> 🎥 The animation file is generated using `tcp-fat-tree-netanim` and shows packet flows and queue dynamics visually. Use it to debug or present flow behaviors under congestion.

---

## 🧠 How It Works (Internals)

| Component           | Description                                               |
|---------------------|-----------------------------------------------------------|
| `swift_cmd.cc`      | Builds fat-tree, configures traffic, runs FlowMonitor     |
| `tcp-swift.cc/h`    | Swift protocol (delay-based AIMD, RTT sampling, pacing)   |
| `tcp-fat-tree-netanim.cc` | Simplified fat-tree generator for NetAnim          |
| `tcp-socket-base.*` | Core TCP logic and socket interface                       |
| `tcp-socket-state.*`| State tracking: cwnd, RTTs, pacing, Swift vars            |

---

## 💬 Contact

Maintainer: **Omar Garah**  
📧 garah@campus.technion.ac.il  
🐙 GitHub: [omarghara](https://github.com/omarghara)

---

## 📚 Citation

If you use this for academic work, please cite:

> Kumar et al., "Swift: Delay is Simple and Effective for Congestion Control in the Datacenter," SIGCOMM 2020.

---
