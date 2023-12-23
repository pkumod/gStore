# SPDX-License-Identifier: BSD-3-Clause

project('gStore', 'cpp',
        version: '1.2',
        license: 'BSD-3-Clause',
        meson_version: '>= 0.62.0',
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


### Dependencies ###
# Managed by Conan
dep_boost = dependency('boost', version: '>= 1.55.0', modules: ['system', 'regex', 'thread'])
dep_curl = dependency('libcurl', version: '>= 6.5.0')
dep_uuid = dependency('uuid')
dep_openssl = dependency('openssl', version: '>= 1.1')
dep_readline = dependency('readline')
dep_zlib = dependency('zlib', version: '>= 1.2')
dep_minizip = dependency('minizip', version: '>= 1.2')

# Managed by system package manager
# In theory we should move dependencies to Conan, but Conan is a world of quirks and imperfections :(
# jemalloc is still breaking: https://github.com/conan-io/conan-center-index/issues/10863
dep_jemalloc = dependency('jemalloc', version: '>= 1.2')
dep_omp = dependency('openmp')
dep_dl = dependency('dl')

cmake = import('cmake')
           