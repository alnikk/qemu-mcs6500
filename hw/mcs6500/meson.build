mcs6500_ss = ss.source_set()

mcs6500_ss.add(files(
  'mcs6500.c',
  'minimal.c'))
mcs6500_ss.add(when: 'CONFIG_NES', if_true: files('nes.c'))

hw_arch += {'mcs6500': mcs6500_ss}
