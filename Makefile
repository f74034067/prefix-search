TESTS = \
    test_cpy \
    test_ref

TEST_INPUT = s Taiwan

CFLAGS = -Wall -Werror -g

# Control the build verbosity                                                   
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied

.PHONY: all clean

all: $(GIT_HOOKS) $(TESTS)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

OBJS_LIB = \
    tst.o

OBJS := \
    $(OBJS_LIB) \
    test_cpy.o \
    test_ref.o

deps := $(OBJS:%.o=.%.o.d)

test_%: test_%.o $(OBJS_LIB)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

clean:
	$(RM) $(TESTS) $(OBJS)
	$(RM) $(deps)

astyle:
	astyle --style=kr --indent=spaces=4 --suffix=none *.[ch]

bench: $(TESTS)
	echo 3 | sudo tee /proc/sys/vm/drop_caches
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./test_cpy --bench $(TEST_INPUT)
	echo 3 | sudo tee /proc/sys/vm/drop_caches
	perf stat --repeat 100 \
		-e cache-misses,cache-references,instructions,cycles \
		./test_ref --bench $(TEST_INPUT)
-include $(deps)
