# Alternative GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug)
  free_list_allocator_config = debug

else ifeq ($(config),release)
  free_list_allocator_config = release

else
  $(error "invalid configuration $(config)")
endif

PROJECTS := free-list-allocator

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

free-list-allocator:
ifneq (,$(free_list_allocator_config))
	@echo "==== Building free-list-allocator ($(free_list_allocator_config)) ===="
	@${MAKE} --no-print-directory -C . -f free-list-allocator.make config=$(free_list_allocator_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f free-list-allocator.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug"
	@echo "  release"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   free-list-allocator"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"