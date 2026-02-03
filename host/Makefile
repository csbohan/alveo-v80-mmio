# Build mmio_example for Alveo V80 (AVED/AMI) on the node.
# AMI is typically under /opt/amd. Adjust AMI_INSTALL if your RPM installs elsewhere.

CC     = gcc
CFLAGS = -Wall -O2

# Default: AMI under /opt/amd (e.g. aved install or RPM)
# Override: make AMI_INSTALL=/usr  or  make AMI_INC=/path/to/ami/include AMI_LIB=/path/to/ami/lib
AMI_INSTALL ?= /opt/amd
AMI_INC     ?= $(AMI_INSTALL)/include
AMI_LIB     ?= $(AMI_INSTALL)/lib

# If your AVED package uses a versioned subdir, set it, e.g.:
# AMI_INSTALL = /opt/amd/aved/amd_v80_gen5x8_23.2_exdes_2_xbtest_stress
# and ensure include/ and lib/ exist under it, or add -I/opt/amd/aved/.../api/include -L.../api/lib

CFLAGS += -I$(AMI_INC)
LDFLAGS = -L$(AMI_LIB) -lami -Wl,-rpath,$(AMI_LIB)

TARGET = mmio_example

.PHONY: all clean

all: $(TARGET)

$(TARGET): mmio_example.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

# Run on node (replace BDF with your card, e.g. 21:00.0)
# sudo ./mmio_example 21:00.0
# With explicit BAR and offset (get offset from xbtest metadata under /opt/amd/aved/.../xbtest/metadata/)
# sudo ./mmio_example 21 0 0x1000
