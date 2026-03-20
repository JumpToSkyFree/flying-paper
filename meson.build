project('flying-paper', 'cpp', default_options : [
  'cpp_std=c++23,c++20',
  'warning_level=3',
  'werror=false'
  ])

cc = meson.get_compiler('cpp')

gtk = dependency('gtk4')
peel = dependency('peel')
libadwaita = dependency('libadwaita-1', version : '>= 1.5', required: true)
tdjson = dependency('tdjson', version : '>= 1.8')
tdclient = dependency('tdclient')

peel_gen = find_program('peel-gen')
peel_gen_command = [
  peel_gen,
  '--recursive',
  '--out-dir', '@OUTDIR@',
  'Gtk', '4.0',
]

if libadwaita.found()
  peel_gen_command += ['Adw', '1']
endif

peel_headers = custom_target('peel-codegen',
  command: peel_gen_command,
  output: 'peel',
)

incdir = include_directories('include')

flying_paper_headers = files([
  './include/Application.hpp',
  './include/ApplicationWindow.hpp',
  './include/PhoneNumberLoginView.hpp',
  './include/Telegram/ClientManager.hpp',
  './include/Telegram/Internal/ClientManagerAccessor.hpp',
  './include/Telegram/Internal/ClientManagerImpl.hpp'
  ])

flying_paper_sources = files([
  './src/Application.cpp',
  './src/ApplicationWindow.cpp',
  './src/PhoneNumberLoginView.cpp',
  './src/Telegram/ClientManager.cpp'
  ])

dependencies = [libadwaita, gtk, peel, tdjson, tdclient, 
  cc.find_library('tdclient',  static: true),
  cc.find_library('tdapi',     static: true),
  cc.find_library('tdcore',    static: true),
  cc.find_library('tdmtproto', static: true),
  cc.find_library('tdnet',     static: true),
  cc.find_library('tddb',      static: true),
  cc.find_library('tdactor',   static: true),
  cc.find_library('tdutils',   static: true),
  cc.find_library('tde2e',     static: true),
  # cc.find_library('tdtl',      static: true),
  # cc.find_library('sqlite',    static: true),
  cc.find_library('tdsqlite', static: true),
  dependency('sqlite3'),
  dependency('openssl'),
  cc.find_library('z'),
  cc.find_library('dl'),
  cc.find_library('pthread'),
  ]

executable('flying-paper',
  'main.cpp',
  flying_paper_headers,
  flying_paper_sources,
  peel_headers,
  include_directories : incdir,
  dependencies : dependencies
)

