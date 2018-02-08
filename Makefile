default: all

# the buildsystem we are using
# possible values: LinuxMakefile
BUILDSYSTEM=LinuxMakefile

# list of projects
ALL_PROJECTS=$(patsubst %/, %, $(dir $(wildcard */*.jucer)))
PROJECTS=$(filter-out _PluginTemplate, $(ALL_PROJECTS))

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
	make -C $(<D)
%-LinuxMakefile-clean: %/Builds/LinuxMakefile/Makefile
	make -C $(<D) clean

%/Builds/LinuxMakefile/Makefile: %.jucer
	$(PROJUCER) -resave "$^"

# XCode based rules
.SECONDEXPANSION:
%-XCode-build: $$(subst @,%,@/Builds/MacOSX/@.xcodeproj/project.pbxproj)
	xcodebuild \
		-project $(<D) \
		-target "$(firstword $(subst /, ,$<)) - All" \
		-configuration "Release" \
		build

# this does not declare a proper dependency,
# so Projucer will be called for each %-build
%.pbxproj:
	$(PROJUCER) -resave $(subst @,$(firstword $(subst /, ,$@)),@/@.jucer)
