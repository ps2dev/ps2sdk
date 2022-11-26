
all: $(PS2SDKSRC)/common/external_deps/lwip

$(PS2SDKSRC)/common/external_deps/lwip:
	cd $(PS2SDKSRC) && ./dowload_dependencies.sh

clean:
	$(ECHO) Cleaning PS2SDK dependencies.
	$(ECHO) Cleaning lwip.
	rm -rf $(PS2SDKSRC)/common/external_deps/lwip

include $(PS2SDKSRC)/Defs.make
