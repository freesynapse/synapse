# Alternative GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug)
  vector_vs_array_speed_config = debug

else ifeq ($(config),release)
  vector_vs_array_speed_config = release

else
  $(error "invalid configuration $(config)")
endif

PROJECTS := vector-vs-array-speed

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

vector-vs-array-speed:
ifneq (,$(vector_vs_array_speed_config))
	@echo "==== Building vector-vs-array-speed ($(vector_vs_array_speed_config)) ===="
	@${MAKE} --no-print-directory -C . -f vector-vs-array-speed.make config=$(vector_vs_array_speed_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f vector-vs-array-speed.make clean

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
	@echo "   vector-vs-array-speed"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"