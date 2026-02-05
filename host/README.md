# Host MMIO examples for Alveo V80 (AVED/AMI)

Run these **on the node** (e.g. `alveo00`) where the V80 and AMI are installed.

## 1. Quick test with ami_tool (no build)

```bash
ami_tool overview                    # get BDF (e.g. 21:00.0)
sudo ami_tool bar_rd -d 21:00.0 -b 0 -a 0x0 -l 4
sudo ami_tool bar_wr -d 21:00.0 -b 0 -a 0x0 -i 0x12345678 -y
```

Use the **offset** from your design or from xbtest metadata under `/opt/amd/aved/.../xbtest/metadata/`.

## 2. Script wrapper (no AMI library)

```bash
chmod +x mmio_ami_tool.sh
sudo ./mmio_ami_tool.sh 21:00.0 0 0x0
```

## 3. C program (AMI API)

Requires AMI headers and library (from `/opt/amd` or your AVED RPM).

```bash
# If AMI is in /opt/amd
make

# If AMI is elsewhere (e.g. RPM put it under /usr)
export AMI_INSTALL=/usr
make

# Run (use your BDF; get BAR offset from xbtest metadata)
sudo ./mmio_example 21:00.0
sudo ./mmio_example 21 0 0x1000
```

If `ami.h` is not found, set `AMI_INC` to the directory that contains the AMI API headers, and `AMI_LIB` to the directory containing `libami.so`:

```bash
make AMI_INC=/path/to/ami/include AMI_LIB=/path/to/ami/lib
```

## Finding BAR offset (pre-built xbtest design)

On the node, the xbtest metadata and card definition describe the BAR layout:

```bash
ls /opt/amd/aved/
# Pick your design dir, e.g. amd_v80_gen5x8_*_xbtest_*
cat /opt/amd/aved/amd_v80_gen5x8_*_xbtest_*/xbtest/metadata/*.json
```

Look for "nominal bar", "offset", or "layout" of xbtest IP in PCIe BAR. That offset is what you pass as the third argument to the script or C example.

## PCA-comp MMIO (test_12x2)

For the PCA-comp design with MMIO bridge (from this repo’s `sbt "runMain pca.PCACompWithMMIO"`), see **[pca_mmio_regs.md](pca_mmio_regs.md)** for the register map and host flow. Use the same AMI pattern as `mmio_example.c` with base offset pointing at the PCA-comp block in your BAR.
