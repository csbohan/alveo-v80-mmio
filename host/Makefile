# Build mmio_example for Alveo V80 (AVED/AMI) on the node.
# AMI is often under /opt/amd/aved/<design>/api/include and api/lib (not /opt/amd/include).

CC     = gcc
CFLAGS = -Wall -O2

# Auto-detect ami.h under /opt/amd (AVED often uses .../api/include/ami.h)
AMI_H      := $(shell find /opt/amd -name 'ami.h' 2>/dev/null | head -1)
AMI_INSTALL ?= /opt/amd
AMI_INC    ?= $(if $(AMI_H),$(dir $(AMI_H)),$(AMI_INSTALL)/include)
AMI_LIB    ?= $(if $(AMI_H),$(patsubst %/include,%/lib,$(dir $(AMI_H))),$(AMI_INSTALL)/lib)

# Override if auto-detect is wrong: make AMI_INC=/path/to/include AMI_LIB=/path/to/lib
# Or set a specific AVED design: make AMI_INSTALL=/opt/amd/aved/amd_v80_gen5x8_xxx_exdes_2_xbtest_stress
# (ensure that path has api/include and api/lib underneath)

CFLAGS += -I$(AMI_INC)
LDFLAGS = -L$(AMI_LIB) -lami -Wl,-rpath,$(AMI_LIB)

TARGET = mmio_example

.PHONY: all clean find-ami

# Run "make find-ami" on the node to see where AMI headers/libs are (or set AMI_INC/AMI_LIB manually)
find-ami:
	@echo "Looking for ami.h under /opt/amd..."
	@F=$$(find /opt/amd -name 'ami.h' 2>/dev/null | head -1); \
	if [ -n "$$F" ]; then \
	  echo "  ami.h: $$F"; \
	  echo "  AMI_INC=$$(dirname $$F)"; \
	  echo "  AMI_LIB=$$(dirname $$(dirname $$F))/lib"; \
	  echo "  Build with: make"; \
	else \
	  echo "  ami.h not found. Install AVED/AMI or set: make AMI_INC=/path/to/include AMI_LIB=/path/to/lib"; \
	fi

all: $(TARGET)

$(TARGET): mmio_example.c
	@echo "Using AMI_INC=$(AMI_INC) AMI_LIB=$(AMI_LIB)"
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

# Run on node (replace BDF with your card, e.g. 21:00.0)
# sudo ./mmio_example 21:00.0
# With explicit BAR and offset (get offset from xbtest metadata under /opt/amd/aved/.../xbtest/metadata/)
# sudo ./mmio_example 21 0 0x1000
