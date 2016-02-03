# Makefile to run simulations and generate statistics at kongull
#
# NOTE: This script assumes that the system is set up as described in
# m5kongull.pdf and that the location of the prefetcher is
# ~/TDT4260/prefetcher/prefetcher.cc.
#
# To set test id and name, set the variables ID and NAME before running make:
# 	ID=123 NAME=prefetcher_with_latency make simulate
#
ID?=0
NAME?="test"

.PHONY: simulate
simulate:
	${HOME}/tools/m5/test_prefetcher.py --id=${ID} --name=${NAME} \
		<${HOME}/TDT4260/prefetcher/prefetcher.cc

.PHONY: stats
stats: simulate
	(cd /work/${USER}/run_${ID}/ && ${HOME}/tools/m5/stats.py) \
		>> ${HOME}/stats.txt
