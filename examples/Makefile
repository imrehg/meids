CPPFLAGS := -I/usr/include/medriver -lpthread
LDFLAGS := -lMEiDS
CFLAGS := -Wall -ggdb

ME_EXAMPLES_LIST := ai_block.tst ai_cb.tst ai_ext_list.tst ai_ext_start.tst ai_ext.tst ai_single.tst ai_range.tst ai_stream.tst ai.tst ai_single_scan.tst \
	ao_cb.tst ao_cb_wraparound.tst ao_single_synch_ext.tst ao_single_synch.tst ao_single.tst \
	ao_stream_synch_ext.tst ao_stream_synch_ext_one.tst ao_stream_synch.tst ao.tst ao_wraparound_synch_ext.tst ao_wraparound_synch.tst \
	cnt.tst geterror.tst irq_cb.tst irq.tst lockAll.tst query.tst query_fast.tst \
	di.tst do.tst curr_single.tst curr_stream.tst \
	aiSingle.tst aoSingle.tst meIOStrFreqToTicks.tst

all: clean examples
examples: ${ME_EXAMPLES_LIST}

meIOStrFreqToTicks.tst: meIOStrFreqToTicks.tst.o
meIOStrFreqToTicks.tst.o: meIOStrFreqToTicks.tst.c

curr_single.tst: curr_single.tst.o
curr_single.tst.o: curr_single.tst.c

curr_stream.tst: curr_stream.tst.o
curr_stream.tst.o: curr_stream.tst.c

ai_block.tst: ai_block.tst.o
ai_block.tst.o: ai_block.tst.c

ai_cb.tst: ai_cb.tst.o
ai_cb.tst.o: ai_cb.tst.c

ai_ext_list.tst: ai_ext_list.tst.o
ai_ext_list.tst.o: ai_ext_list.tst.c

ai_ext_start.tst: ai_ext_start.tst.o
ai_ext_start.tst.o: ai_ext_start.tst.c

ai_ext.tst: ai_ext.tst.o
ai_ext.tst.o: ai_ext.tst.c

ai_single.tst: ai_single.tst.o
ai_single.tst.o: ai_single.tst.c

ai_range.tst: ai_range.tst.o
ai_range.tst.o: ai_range.tst.c

ai.tst: ai.tst.o
ai.tst.o: ai.tst.c

ai_stream.tst: ai_stream.tst.o
ai_stream.tst.o: ai_stream.tst.c

ai_single_scan.tst: ai_single_scan.tst.o
ai_single_scan.tst.o: ai_single_scan.tst.c

ao_cb.tst: ao_cb.tst.o
ao_cb.tst.o: ao_cb.tst.c

ao_cb_wraparound.tst: ao_cb_wraparound.tst.o
ao_cb_wraparound.tst.o: ao_cb_wraparound.tst.c

ao_single_synch_ext.tst: ao_single_synch_ext.tst.o
ao_single_synch_ext.tst.o: ao_single_synch_ext.tst.c

ao_single_synch.tst: ao_single_synch.tst.o
ao_single_synch.tst.o: ao_single_synch.tst.c

ao_single.tst: ao_single.tst.o
ao_single.tst.o: ao_single.tst.c

ao_stream_synch_ext.tst: ao_stream_synch_ext.tst.o
ao_stream_synch_ext.tst.o: ao_stream_synch_ext.tst.c

ao_stream_synch_ext_one.tst: ao_stream_synch_ext_one.tst.o
ao_stream_synch_ext_one.tst.o: ao_stream_synch_ext_one.tst.c

ao_stream_synch.tst: ao_stream_synch.tst.o
ao_stream_synch.tst.o: ao_stream_synch.tst.c

ao.tst: ao.tst.o
ao.tst.o: ao.tst.c

ao_wraparound_synch_ext.tst: ao_wraparound_synch_ext.tst.o
ao_wraparound_synch_ext.tst.o: ao_wraparound_synch_ext.tst.c

ao_wraparound_synch.tst: ao_wraparound_synch.tst.o
ao_wraparound_synch.tst.o: ao_wraparound_synch.tst.c

cnt.tst: cnt.tst.o
cnt.tst.o: cnt.tst.c

cnt.tst: cnt.tst.o
cnt.tst.o: cnt.tst.c

geterror.tst: geterror.tst.o
geterror.tst.o: geterror.tst.c

irq.tst: irq.tst.o
irq.tst.o: irq.tst.c

irq_cb.tst: irq_cb.tst.o
irq_cb.tst.o: irq_cb.tst.c

lockAll.tst: lockAll.tst.o
lockAll.tst.o: lockAll.tst.c

query.tst: query.tst.o
query.tst.o: query.tst.c

di.tst: di.tst.o
di.tst.o: di.tst.c

do.tst: do.tst.o
do.tst.o: do.tst.c

aiSingle.tst: aiSingle.tst.o
aiSingle.tst.o: aiSingle.tst.c

aoSingle.tst: aoSingle.tst.o
aoSingle.tst.o: aoSingle.tst.c

query_fast.tst: query_fast.tst.o
query_fast.tst.o: query_fast.tst.c

# Special builds
ai_single_CQ: ai_single_CQ.tst
ai_single_CQ.tst: ai_single_CQ.tst.o
ai_single_CQ.tst.o: ai_single_CQ.tst.c

MephistoScopeStreamRead.tst: MephistoScopeStreamRead.tst.o
MephistoScopeStreamRead.tst.o: MephistoScopeStreamRead.tst.c

install:
	su -c "cp -f *.tst /usr/local/bin"

uninstall:
	su -c "cd /usr/local/bin/; rm -f ${ME_EXAMPLES_LIST}"

clear:
	rm -f ${ME_EXAMPLES_LIST}

clean:
	rm -f *.o *.swp *~
