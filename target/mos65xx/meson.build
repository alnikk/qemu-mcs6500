mos65xx_ss = ss.source_set()
mos65xx_softmmu_ss = ss.source_set()

mos65xx_ss.add(files(
  'cpu.c',
  'helper.c',
  'gdbstub.c'))

target_arch += {'mos65xx': mos65xx_ss}
target_softmmu_arch += {'mos65xx': mos65xx_softmmu_ss}
