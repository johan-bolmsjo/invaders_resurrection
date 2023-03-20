# Include this file last in the top level makefile.
# It's important for dependencies to work properly!

# Rule that show all PHONY targets that a user typically is interested in running.
.PHONY: help
PHONY_TARGETS += help
help:
	@echo "Make targets:"
	@for target in $(sort $(PHONY_TARGETS)); do \
	    echo "  $$target" ;                     \
	done

# Rule that compile all test executables
.PHONY: test
PHONY_TARGETS += test
test: $(call expand-targets-test)

# Rule that run all test executables
.PHONY: runtest
PHONY_TARGETS += runtest
runtest: $(call expand-targets-test)
	@for test in $(call expand-targets-test); do \
	    echo "Executing $$test" ;         \
	    $$test || exit 1;                 \
	done

# Trigger rebuild of bulid products that are older than any makefile.
$(call expand-targets-all) $(call expand-objs-all): $(filter-out $(OUTDIR_CHECK)/%,$(MAKEFILE_LIST))

# Make all build products depend on output directories by order only prerequisites.
$(foreach target,$(call expand-targets-all) $(call expand-objs-all),$(eval $(target): |$(dir $(target))))

# Rule that creates output directories.
$(sort $(dir $(call expand-targets-all) $(call expand-objs-all))):
	$(Q)mkdir -p $@

# Read dependency info for *existing* .o files.
# The `help' guard is not perfect but prevents rebuilding generated sources that
# dependency files depend on when the help command is invoked.
ifneq ($(filter-out help,$(MAKECMDGOALS)),)
-include $(patsubst %.o,%.d, $(call expand-objs-all))
endif
