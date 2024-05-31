
LWIP := $(PS2SDKSRC)/common/external_deps/lwip
LIBSMB2 := $(PS2SDKSRC)/common/external_deps/libsmb2

all: $(LWIP)

$(LWIP):
	cd $(PS2SDKSRC) && ./download_dependencies.sh

clean:
	@$(PRINTF) 'Cleaning PS2SDK dependencies.\n'
	@$(PRINTF) 'Cleaning lwip.\n'
	rm -rf $(LWIP) $(LWIP)_inprogress
	@$(PRINTF) 'Cleaning libsmb2.\n'
	rm -rf $(LIBSMB2) $(LIBSMB2)_inprogress

release:
	@cd .

include $(PS2SDKSRC)/Defs.make
