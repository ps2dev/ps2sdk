
LWIP := $(PS2SDKSRC)/common/external_deps/lwip

all: $(LWIP)

$(LWIP):
	cd $(PS2SDKSRC) && ./download_dependencies.sh

clean:
	@$(PRINTF) 'Cleaning PS2SDK dependencies.\n'
	@$(PRINTF) 'Cleaning lwip.\n'
	rm -rf $(LWIP) $(LWIP)_inprogress

include $(PS2SDKSRC)/Defs.make
