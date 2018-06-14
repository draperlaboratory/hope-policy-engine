.PHONY: all
.PHONY: install
.PHONY: clean

ISP_PREFIX ?= /opt/isp/

all: build/Makefile
	$(MAKE) -C build

build/Makefile: policy
	$(RM) -r build
	mkdir -p build
	cd build; cmake ..

policy:
	./build_policy

install: all
	install -d $(ISP_PREFIX)/lib
	install -p build/md_code $(ISP_PREFIX)/bin/
	install -p build/md_range $(ISP_PREFIX)/bin/
	install -p build/standalone $(ISP_PREFIX)/bin/
	install -p tagging_tools/gen_tag_info $(ISP_PREFIX)/bin/
	install -p scripts/run_riscv $(ISP_PREFIX)/bin/
	install -p scripts/run_riscv_gdb $(ISP_PREFIX)/bin/
	install -p build/librv32-renode-validator.so $(ISP_PREFIX)/lib/
	install -p build/librv32_validator.a $(ISP_PREFIX)/lib/
	install -p build/libtagging_tools.a $(ISP_PREFIX)/lib/
	install -p build/libvalidator.a $(ISP_PREFIX)/lib/
	cp -r policy $(ISP_PREFIX)/

clean:
	$(RM) -r build
	$(RM) -r policy
