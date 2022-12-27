
LWIP := $(PS2SDKSRC)/common/external_deps/lwip

all: $(LWIP)

$(LWIP):
	cd $(PS2SDKSRC) && ./dowload_dependencies.sh

clean:
	$(ECHO) Cleaning PS2SDK dependencies.
	$(ECHO) Cleaning lwip.
	rm -rf $(LWIP) $(LWIP)_inprogress

include $(PS2SDKSRC)/Defs.make
