all: $(DEFAULT_COMPONENTS)

ALL_COMPONENTS = src ctf test_model pgemm_test nonsq_pgemm_test bench \
                 bench_model nonsq_pgemm_bench examples fft fft_3D gemm \
                 gemm_4D trace sym3 ccsdt_t3_to_t2 weight_4D

examples: fft fft_3D gemm trace sym3 ccsdt_t3_to_t2 weight_4D
test_model bench_model pgemm_test nonsq_pgemm_test: ctf
nonsq_pgemm_bench fft fft_3D gemm gemm_4D trace sym3: ctf 
ccsdt_t3_to_t2 weight_4D: ctf

bindir = ${top_dir}/bin
libdir = ${top_dir}/lib

DEPDIR = .deps
DEPS += ${top_dir}/.dummy $(addprefix $(DEPDIR)/,$(notdir $(patsubst %.o,%.Po,$(wildcard *.o))))
ALL_SUBDIRS = $(sort $(SUBDIRS) $(foreach comp,$(ALL_COMPONENTS),$(value $(addsuffix _SUBDIRS,$(comp)))))

_INCLUDES = $(INCLUDES) -I. -I${top_dir} -I${top_dir}/include
_CXXFLAGS = $(CXXFLAGS)
_DEFS = $(DEFS)
_LDFLAGS = $(LDFLAGS) -L${top_dir}/lib
_DEPENDENCIES = $(DEPENDENCIES) Makefile ${top_dir}/config.mk ${top_dir}/rules.mk
_LIBS = $(LIBS)
pgemm_test nonsq_pgemm_test nonsq_pgemm_bench : _LIBS = $(SCALA) $(LIBS)

CXXCOMPILE = $(CXX) $(DEFS) $(_INCLUDES) $(_CPPFLAGS) $(_CXXFLAGS)
CXXCOMPILEDEPS = $(CXXCOMPILE) $(DEPFLAGS)
LINK = $(CXX) $(_CXXFLAGS) $(_LDFLAGS) -o $@
ARCHIVE = $(AR) $@

#
# Automatic library dependency generation derived from: Steve Dieters
# http://lists.gnu.org/archive/html/help-make/2010-03/msg00072.html
#
libdeps = \
$(foreach d, $(patsubst -L%,%,$(filter -L%,$(1))),\
 $(foreach l, $(patsubst -l%,%,$(filter -l%,$(1))),\
  $(if $(shell if [ -e $(d)/lib$(l).a ]; then echo "X"; fi),\
   $(d)/lib$(l).a\
  )\
 )\
) $(filter %.a,$(1))



.NOTPARALLEL:
.PHONY: all default clean $(ALL_COMPONENTS)
FORCE:

$(ALL_COMPONENTS):
	@for dir in $(SUBDIRS) $($@_SUBDIRS); do \
		echo "Making $@ in $$dir"; \
		(cd $$dir && $(MAKE) $@); \
	done

clean:
	rm -rf $(DEPDIR) *.o
	@for subdir in $(ALL_SUBDIRS); do \
		(cd $$subdir && $(MAKE) clean); \
	done

$(bindir)/%: $(_DEPENDENCIES) $(call libdeps,$(_LIBS))
	@mkdir -p $(dir $@)
	$(LINK) $(filter %.o,$^) $(FLIBS) $(_LIBS)

$(libdir)/%: $(_DEPENDENCIES)
	@mkdir -p $(dir $@)
	$(ARCHIVE) $(filter %.o,$^)

%.o: %.cxx $(_DEPENDENCIES)
	@mkdir -p $(DEPDIR)
	$(CXXCOMPILEDEPS) -c -o $@ $<

-include $(DEPS)
