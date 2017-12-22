default: all

# the buildsystem we are using
# possible values: LinuxMakefile
BUILDSYSTEM=LinuxMakefile

# list of projects
PROJECTS=$(patsubst %/, %, $(dir $(wildcard */*.jucer)))

# helper applications
PROJUCER=Projucer
TOUCH=touch

# generic rules
.PHONY: distclean clean all

noop=
space=$(noop) $(noop)
vpath %.jucer $(subst $(space),:,$(PROJECTS))

all: $(PROJECTS:%=%-$(BUILDSYSTEM)-build)
clean: $(PROJECTS:%=%-$(BUILDSYSTEM)-clean)
distclean:
	rm -rf */Builds
# aliases
$(PROJECTS:%=%-build):
	make $(@:%-build=%)-$(BUILDSYSTEM)-build
$(PROJECTS:%=%-clean):
	make $(@:%-clean=%)-$(BUILDSYSTEM)-clean

# LinuxMakefile based rules
%-LinuxMakefile-build: %/Builds/LinuxMakefile/Makefile
	make -C $(<D)
%-LinuxMakefile-clean: %/Builds/LinuxMakefile/Makefile
	make -C $(<D) clean

%/Builds/LinuxMakefile/Makefile: %.jucer
	$(PROJUCER) -resave "$^"
	$(TOUCH) "$@"
