
LWIP := $(PS2SDKSRC)/common/external_deps/lwip
FATFS := $(PS2SDKSRC)/common/external_deps/fatfs

all: 
	cd $(PS2SDKSRC) && ./download_dependencies.sh

clean:
	@$(PRINTF) 'Cleaning PS2SDK dependencies.\n'
	@$(PRINTF) 'Cleaning lwip.\n'
	rm -rf $(LWIP) $(LWIP)_inprogress
	@$(PRINTF) 'Cleaning fatfs.\n'
	rm -rf $(FATFS) $(FATFS)_inprogress

release:
	@cd .

include $(PS2SDKSRC)/Defs.make
