# Alveo V80: Host MMIO and FPGA AXI Bridge

## Important: V80 uses AVED/AMI, not XRT

Alveo V80 does **not** use Xilinx Runtime (XRT). It uses the **AVED Management Interface (AMI)**. You cannot use XRT APIs (`xrt::kernel::read_register` / `write_register`) on V80. Use AMI’s BAR read/write instead.

---

## 1. Host side: MMIO from CPU

### Option A: Command line (quick test)

Use `ami_tool bar_rd` and `ami_tool bar_wr` (requires root/sudo):

```bash
# Get your card BDF first (e.g. from ami_tool overview)
ami_tool overview

# Read one 32-bit register at BAR 0, offset 0x1000
sudo ami_tool bar_rd -d <BDF> -b 0 -a 0x1000

# Read 10 registers starting at offset 0x1000
sudo ami_tool bar_rd -d <BDF> -b 0 -a 0x1000 -l 10

# Write one 32-bit value to BAR 0, offset 0x1000
sudo ami_tool bar_wr -d <BDF> -b 0 -a 0x1000 -i 0x22
```

Replace `<BDF>` with your device (e.g. `21:00.0` or `21`). BAR index is usually **0** for PF0. The **offset** must match your FPGA AXI-Lite address map (see below).

### Option B: C code using AMI API

Use the AMI API from your own program:

- `ami_dev_find(bdf, &dev)` – open device by BDF
- `ami_dev_request_access(dev)` – required for BAR access (root)
- `ami_mem_bar_read(dev, bar_idx, offset, &val)` – read 32-bit register
- `ami_mem_bar_write(dev, bar_idx, offset, val)` – write 32-bit register
- `ami_dev_delete(&dev)` – close device

See `host/mmio_example.c` and `host/Makefile` in this repo. Build and run on the node (link against AMI; AMI is under `/opt/amd` or your RPM install).

**Build on the node:**
```bash
cd host
# If AMI is not under /opt/amd, set AMI_INSTALL, e.g.:
# export AMI_INSTALL=/usr   # if RPM installed to system
make
sudo ./mmio_example 21:00.0          # use your BDF from ami_tool overview
sudo ./mmio_example 21 0 0x1000      # BAR 0, offset 0x1000 (get offset from xbtest metadata)
```

If you don't have the AMI development library, use the script that wraps `ami_tool`:
```bash
sudo ./mmio_ami_tool.sh 21:00.0 0 0x1000
```

---

## 2. Finding the BAR offset (pre-built xbtest design)

The pre-built AVED design includes **xbtest IP** with scratch registers. The verify test uses these. The offset is card-design specific:

- **Card definition / xbtest metadata**  
  On the node, look under your AVED install, e.g.  
  `/opt/amd/aved/amd_v80_gen5x8_*_xbtest_*/xbtest/metadata/`  
  for JSON that describes “nominal bar, offset” and “layout of xbtest hardware IP in PCIe BAR”. That gives you the BAR index and base offset for the xbtest registers.

- **AVED memory map**  
  [AVED V80 Memory Map](https://xilinx.github.io/AVED/amd_v80_gen5x8_exdes_2_20240408/AVED+V80+-+Memory+Map.html) (PL Memory Space) shows how address space is carved; your AXI peripheral’s base address in that map (after any remapping) corresponds to the BAR offset you use with `bar_rd` / `bar_wr` and the AMI API.

If you only want to **test host MMIO** with the current image, use the xbtest scratch register offset from that metadata and talk to it with `ami_tool` or the example C code.

---

## 3. FPGA side: Your own AXI “bridge” (registers)

To have **your own** registers readable/writable from the host:

1. **Add an AXI-Lite peripheral** in the AVED design (e.g. AXI4-Lite GPIO or a small custom AXI-Lite slave with a few 32-bit registers).
2. **Connect it** to the same AXI path that goes to PCIe (e.g. CIPS/NoC so it appears in PF0 BAR space). See AVED “Host to Card Communication” and “Memory Map” for the data path.
3. **Assign a base address** in the PL memory map; that address (after remapping) is the **BAR offset** you use on the host.
4. **Rebuild** the AVED design in Vivado and **re-flash** the card (or program the PDI via `ami_tool cfgmem_program` as per [AVED docs](https://xilinx.github.io/AVED/latest/AVED+Updating+FPT+Image+in+Flash.html)).

The pre-built design does not include your custom IP; it only has the xbtest IP. So “writing/reading FPGA register values from CPU” with **your** registers requires this FPGA design change. Until then, you can still do MMIO to the **existing** xbtest scratch registers using the offsets from the metadata.

---

## 4. Summary: What to do on the node

1. **Quick MMIO test**  
   - Run `ami_tool overview` to get BDF.  
   - Find BAR and offset from xbtest metadata under `/opt/amd/aved/.../xbtest/metadata/`.  
   - Use `sudo ami_tool bar_rd` / `bar_wr` with that BDF, BAR, and offset.

2. **MMIO from your own program**  
   - Use the C example in `host/mmio_example.c` (build with AMI lib/include from `/opt/amd` or your RPM).  
   - Same BDF and BAR/offset as above; run with sudo if you call `ami_dev_request_access()`.

3. **Your own FPGA registers**  
   - Add an AXI-Lite block in the AVED design, wire it into the PCIe-visible address space, assign a base address, rebuild, and re-flash. Then use that base as the BAR offset in `ami_tool` or the AMI API.

---

## References

- [AVED Management Interface (ami_tool) userguide](https://xilinx.github.io/AVED/amd_v80_gen5x8_exdes_2_20240408/AVED+Management+Interface+userguide+(ami_tool).html) – `bar_rd` / `bar_wr`
- [AMI API Description](https://xilinx.github.io/AVED/amd_v80_gen5x8_exdes_1_20231204/AMI+-+API+Description.html) – `ami_mem_bar_read` / `ami_mem_bar_write`
- [AVED Host to Card Communication](https://xilinx.github.io/AVED/amd_v80_gen5x8_exdes_2_20240408/AVED+-+Host+to+Card+Communication.html)
- [AVED V80 Memory Map](https://xilinx.github.io/AVED/amd_v80_gen5x8_exdes_2_20240408/AVED+V80+-+Memory+Map.html)
