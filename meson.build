project('devutils', 'c',
  version: '1.0.0',
  license: 'GPL-3.0-or-later',
  meson_version: '>= 0.55.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'optimization=2'
  ]
)

# Project information
project_name = meson.project_name()
project_version = meson.project_version()

# Installation directories
prefix = get_option('prefix')
bindir = get_option('bindir')
mandir = get_option('mandir')

# Compiler
cc = meson.get_compiler('c')

# Compiler flags
c_args = []
link_args = []

# Handle build options - only add flags not handled by built-in options
if get_option('enable_debug') or get_option('buildtype') == 'debug'
  c_args += ['-DDEBUG']
else
  c_args += ['-DNDEBUG']
endif

# Sanitizer support
sanitizer = get_option('sanitizer')
if sanitizer != 'none'
  sanitizer_args = ['-fsanitize=' + sanitizer]
  c_args += sanitizer_args
  link_args += sanitizer_args
endif

# Static linking
if get_option('static')
  link_args += ['-static']
endif

# Add compiler-specific flags (only those not covered by warning_level)
if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  c_args += [
    '-Wformat=2',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes'
  ]
endif

# Apply flags globally
add_project_arguments(c_args, language: 'c')
add_project_link_arguments(link_args, language: 'c')

# Dependencies
# Add any external dependencies here if needed
deps = []

# Include directories
inc = include_directories('src/include')

# Configuration data
conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', project_name)
conf.set_quoted('PACKAGE_VERSION', project_version)
conf.set_quoted('PREFIX', prefix)
conf.set_quoted('BINDIR', bindir)
conf.set_quoted('MANDIR', mandir)

# Generate config.h
config_h = configure_file(
  output: 'config.h',
  configuration: conf
)

# Add subdirectories
subdir('src')
subdir('doc')

# Summary
summary({
  'Package': project_name,
  'Version': project_version,
  'License': 'GPL-3.0-or-later',
  'Prefix': prefix,
  'Binary directory': bindir,
  'Manual directory': mandir,
}, section: 'Configuration')

summary({
  'C compiler': cc.get_id(),
  'C standard': get_option('c_std'),
  'Warning level': get_option('warning_level'),
  'Optimization': get_option('optimization'),
}, section: 'Build options')