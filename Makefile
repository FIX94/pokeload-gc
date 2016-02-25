# Copyright 2016 FIX94
# This code is licensed to you under the terms of the GNU LGPL, version 3;
# see file LICENSE or http://www.gnu.org/licenses/lgpl-3.0.txt

all:
	@$(MAKE) --no-print-directory -C loader
	@mv -f loader/loader.h exploit/loader.h
	@$(MAKE) --no-print-directory -C exploit
	@mkdir -p gci
	@mv -f exploit/*.gci gci

clean:
	@$(MAKE) --no-print-directory -C loader clean
	@$(MAKE) --no-print-directory -C exploit clean
	rm -rf gci
