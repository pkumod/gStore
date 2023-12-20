# SPDX-License-Identifier: BSD-3-Clause

project('gStore', 'cpp',
        version: '1.2',
        license: 'BSD-3-Clause',
        meson_version: '>= 0.51.0',
        default_options: ['cpp_std=c++11', 'warning_level=3', 'buildtype=debugoptimized'],
)

# Compiler checks
cpp = meson.get_compiler('cpp')
if cpp.get_id() != 'gcc'
    error('gStore only supports gcc compiler. If you have it on your system, run `CC=gcc CXX=g++ meson ...`')
endif
if cpp.version().version_compare('>= 5.4') == false
    error('gStore requires gcc version >= 5.4')
endif
# Check if ccache is available; if not, issue an warning
ccache = find_program('ccache')
if ccache.found() == false
    warning('we recommend using ccache to speed up the compile, run `sudo apt install ccache`')
endif


# flags
obj_flags = ['-Wall', '-O2', '-Werror=return-type']
exe_flags = ['-Wall', '-O2', '-Werror=return-type']


# Dependencies
dep_boost = dependency('boost', version: '>= 1.54.0', modules: ['system', 'regex', 'thread'])
dep_curl = dependency('libcurl', version: '>= 7.0.0')
dep_uuid = dependency('uuid')
dep_openssl = dependency('openssl', version: '>= 1.1')
dep_readline = dependency('readline')
dep_zlib = dependency('zlib', version: '>= 1.2')
dep_jemalloc = dependency('jemalloc', version: '>= 1.2')
dep_realpath = dependency('realpath')
dep_omp = dependency('openmp')
dep_minizip = dependency('minizip', version: '>= 1.2')
dep_dl = dependency('dl')

# build antlr4
cmake = import('cmake')
           