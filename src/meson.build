# Source files for each utility
countfile_sources = files('countfile.c')
cloc_sources = files('cloc.c') 
hexdump_sources = files('hexdump.c')
checksum_sources = files('checksum.c')
diff_sources = files('diff.c')

# Common compile arguments for library builds
lib_c_args = []
if get_option('lib_only')
  lib_c_args += ['-DCOUNTFILE_LIB_ONLY', '-DCLOC_LIB_ONLY']
endif

# Executable targets (only if not lib_only)
executables = []

if not get_option('lib_only')
  # Build countfile utility
  countfile_exe = executable('countfile',
    countfile_sources,
    include_directories: inc,
    dependencies: deps,
    c_args: lib_c_args,
    install: true,
    install_dir: bindir
  )
  executables += [countfile_exe]

  # Build cloc utility  
  cloc_exe = executable('cloc',
    cloc_sources,
    include_directories: inc,
    dependencies: deps,
    c_args: lib_c_args,
    install: true,
    install_dir: bindir
  )
  executables += [cloc_exe]

  # Build hexdump utility
  hexdump_exe = executable('hexdump',
    hexdump_sources,
    include_directories: inc,
    dependencies: deps,
    install: true,
    install_dir: bindir
  )
  executables += [hexdump_exe]

  # Build checksum utility
  checksum_exe = executable('checksum',
    checksum_sources,
    include_directories: inc,
    dependencies: deps,
    install: true,
    install_dir: bindir
  )
  executables += [checksum_exe]

  # Build diff utility
  diff_exe = executable('diff',
    diff_sources,
    include_directories: inc,
    dependencies: deps,
    install: true,
    install_dir: bindir
  )
  executables += [diff_exe]
endif

# Optional: Build as libraries for external use
countfile_lib = static_library('countfile',
  countfile_sources,
  include_directories: inc,
  dependencies: deps,
  c_args: lib_c_args + ['-DCOUNTFILE_LIB_ONLY'],
  install: get_option('lib_only')
)

cloc_lib = static_library('cloc',
  cloc_sources,
  include_directories: inc, 
  dependencies: deps,
  c_args: lib_c_args + ['-DCLOC_LIB_ONLY'],
  install: get_option('lib_only')
)

# Optional: Create aliases for common targets
if executables.length() > 0
  alias_target('devutils_all', executables)
endif

# Test executables (if you want to run basic tests and tests are enabled)
if get_option('tests') and not get_option('lib_only')
  if executables.length() > 0
    test('countfile_basic', executables[0], args: ['--help'], should_fail: false, timeout: 10)
    test('cloc_basic', executables[1], args: [], should_fail: true, timeout: 10)  # Should fail without arguments
    test('hexdump_basic', executables[2], args: ['--help'], should_fail: false, timeout: 10)
    test('checksum_basic', executables[3], args: ['--help'], should_fail: false, timeout: 10)
    test('diff_basic', executables[4], args: ['--help'], should_fail: false, timeout: 10)
  endif
endif