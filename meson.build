project('flying-paper', 'cpp', default_options : [
  'cpp_std=c++23,c++20',
  'warning_level=3',
  'werror=false'
  ])

cc = meson.get_compiler('cpp')

gtk = dependency('gtk4')
peel = dependency('peel')
libadwaita = dependency('libadwaita-1', version : '>= 1.5', required : true)
pixbuf = dependency('gdk-pixbuf-2.0', version : '>= 2.0', required : true)

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
  './include/Views/PhoneNumberLoginView.hpp',
  './include/Views/VerificationCodeInsertView.hpp',
  './include/Views/SidebarView.hpp',
  './include/Telegram/ClientManager.hpp',
  './include/Telegram/Internal/ClientManagerAccessor.hpp',
  './include/Telegram/Internal/ClientManagerImpl.hpp',
  './include/Telegram/Session.hpp',
  './include/Widgets/Avatar.hpp',
  './include/Widgets/ChatListItem.hpp',
  './include/Widgets/DateTime.hpp',
  './include/Widgets/LastMessage.hpp',
  './include/Widgets/ScrolledContainer.hpp'
  ])

flying_paper_sources = files([
  './src/Application.cpp',
  './src/ApplicationWindow.cpp',
  './src/Telegram/ClientManager.cpp',
  './src/Views/PhoneNumberLoginView.cpp',
  './src/Views/SidebarView.cpp',
  './src/Views/VerificationCodeInsertView.cpp',
  './src/Telegram/Session.cpp',
  './src/Widgets/Avatar.cpp',
  './src/Widgets/ChatListItem.cpp',
  './src/Widgets/DateTime.cpp',
  './src/Widgets/LastMessage.cpp',
  './src/Widgets/ScrolledContainer.cpp'
  ])

dependencies = [
  libadwaita,
  gtk,
  peel,
  pixbuf,
  cc.find_library('tdclient',  static: true),
  cc.find_library('tdapi',     static: true),
  cc.find_library('tdcore',    static: true),
  cc.find_library('tdmtproto', static: true),
  cc.find_library('tdnet',     static: true),
  cc.find_library('tddb',      static: true),
  cc.find_library('tdactor',   static: true),
  cc.find_library('tdutils',   static: true),
  cc.find_library('tde2e',     static: true),
  cc.find_library('tdsqlite',  static: true),
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

