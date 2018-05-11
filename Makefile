default: all

# the buildsystem we are using
# possible values: LinuxMakefile XCode
uname := $(shell uname)

ifeq ($(findstring $(uname), Linux GNU GNU/kFreeBSD), $(uname))
  BUILDSYSTEM = LinuxMakefile
endif

ifeq ($(findstring MSYS, $(uname)), MSYS)
  BUILDSYSTEM = VS2017
  BITS = 64
endif

ifeq ($(uname), Darwin)
  BUILDSYSTEM = XCode
endif

CONFIG = Release

system:
	@echo "uname : $(uname)"
	@echo "system: $(BUILDSYSTEM)"
	@echo "config: $(CONFIG)"

# list of projects
ALL_PROJECTS=$(patsubst %/, %, $(dir $(wildcard */*.jucer)))
PROJECTS=$(filter-out _PluginTemplate, $(ALL_PROJECTS))


# what do we want to build: Standalone, VST; all
## currently this has only an effect on the Linux buildsystem
TARGET=all

# helper applications
PROJUCER=Projucer

# generic rules
.PHONY: distclean clean all

noop=
space=$(noop) $(noop)
vpath %.jucer $(subst $(space),:,$(ALL_PROJECTS))

all: $(PROJECTS:%=%-$(BUILDSYSTEM)-build)
clean: $(PROJECTS:%=%-$(BUILDSYSTEM)-clean)
distclean:
	rm -rf */Builds
# aliases
$(ALL_PROJECTS:%=%-build):
	make $(@:%-build=%)-$(BUILDSYSTEM)-build
$(ALL_PROJECTS:%=%-clean):
	make $(@:%-clean=%)-$(BUILDSYSTEM)-clean

# LinuxMakefile based rules
%-LinuxMakefile-build: %/Builds/LinuxMakefile/Makefile
	make -C $(<D) \
		CONFIG="$(CONFIG)" \
		$(TARGET)
%-LinuxMakefile-clean: %/Builds/LinuxMakefile/Makefile
	make -C $(<D) \
		CONFIG="$(CONFIG)" \
		clean

%/Builds/LinuxMakefile/Makefile: %.jucer
	$(PROJUCER) -resave "$^"

# XCode based rules
.SECONDEXPANSION:
%-XCode-build: $$(subst @,%,@/Builds/MacOSX/@.xcodeproj/project.pbxproj)
	xcodebuild \
		-project $(<D) \
		-target "$(firstword $(subst /, ,$<)) - All" \
		-configuration "$(CONFIG)" \
		build
%-XCode-clean: $$(subst @,%,@/Builds/MacOSX/@.xcodeproj/project.pbxproj)
	xcodebuild \
		-project $(<D) \
		-target "$(firstword $(subst /, ,$<)) - All" \
		-configuration "$(CONFIG)" \
		clean


# this does not declare a proper dependency,
# so Projucer will be called for each %-build
%.pbxproj:
	$(PROJUCER) -resave $(subst @,$(firstword $(subst /, ,$@)),@/@.jucer)


# VS2017 based rules
# TODO: find out how to pass CONFIG and TARGET
%-VS2017-build: $$(subst @,%,@/Builds/VisualStudio2017/@.sln)
	MSBuild.exe \
		$<
%-VS2017-clean: $$(subst @,%,@/Builds/VisualStudio2017/@.sln)
	MSBuild.exe \
		-t:Clean \
		$<


# this does not declare a proper dependency,
# so Projucer will be called for each %-build
%.sln:
	$(PROJUCER) -resave $(subst @,$(firstword $(subst /, ,$@)),@/@.jucer)

