AR = '/usr/bin/ar'
ARFLAGS = 'rcs'
BINDIR = '/usr/local/bin'
CC = ['/usr/bin/gcc']
CCLNK_SRC_F = []
CCLNK_TGT_F = ['-o']
CC_NAME = 'gcc'
CC_SRC_F = []
CC_TGT_F = ['-c', '-o']
CC_VERSION = ('4', '9', '2')
CFLAGS = ['-I/home/pepper/LV2_render', '-DNDEBUG', '-fshow-column', '-std=c99']
CFLAGS_MACBUNDLE = ['-fPIC']
CFLAGS_cshlib = ['-fPIC']
CHECKED_JACK = 2
CHECKED_LILV = 2
CHECKED_LV2 = 2
CHECKED_SERD = 2
CHECKED_SORD = 2
CHECKED_SRATOM = 2
CHECKED_SUIL = 2
COMPILER_CC = 'gcc'
COMPILER_CXX = 'g++'
CPPPATH_ST = '-I%s'
CXX = ['/usr/bin/g++']
CXXFLAGS = ['-I/home/pepper/LV2_render', '-DNDEBUG', '-fshow-column']
CXXFLAGS_MACBUNDLE = ['-fPIC']
CXXFLAGS_cxxshlib = ['-fPIC']
CXXLNK_SRC_F = []
CXXLNK_TGT_F = ['-o']
CXX_NAME = 'gcc'
CXX_SRC_F = []
CXX_TGT_F = ['-c', '-o']
DATADIR = '/usr/local/share'
DEBUG = False
DEFINES = ['HAVE_LV2=1', 'HAVE_LILV=1', 'HAVE_SERD=1', 'HAVE_SORD=1', 'HAVE_SUIL=1', 'HAVE_SRATOM=1', 'HAVE_JACK=1', 'HAVE_JACK_PORT_TYPE_GET_BUFFER_SIZE=1', 'HAVE_JACK_METADATA=1', 'JALV_JACK_SESSION=1', 'JALV_VERSION="1.4.6"']
DEFINES_ST = '-D%s'
DEST_BINFMT = 'elf'
DEST_CPU = 'x86_64'
DEST_OS = 'linux'
DOCDIR = '/usr/local/share/doc'
DOCS = False
INCLUDEDIR = '/usr/local/include'
INCLUDES_LILV = ['/usr/include/lilv-0', '/usr/include/sratom-0', '/usr/include/sord-0', '/usr/include/serd-0']
INCLUDES_SERD = ['/usr/include/serd-0']
INCLUDES_SORD = ['/usr/include/sord-0', '/usr/include/serd-0']
INCLUDES_SRATOM = ['/usr/include/sratom-0', '/usr/include/sord-0', '/usr/include/serd-0']
INCLUDES_SUIL = ['/usr/include/suil-0']
JALV_JACK_SESSION = 1
JALV_VERSION = '1.4.6'
LIB = ['m', 'sndfile']
LIBDIR = '/usr/local/lib'
LIBPATH_ST = '-L%s'
LIB_JACK = ['jack']
LIB_LILV = ['lilv-0', 'dl', 'sratom-0', 'sord-0', 'serd-0']
LIB_SERD = ['serd-0']
LIB_SORD = ['sord-0', 'serd-0']
LIB_SRATOM = ['sratom-0', 'sord-0', 'serd-0']
LIB_ST = '-l%s'
LIB_SUIL = ['suil-0']
LINKFLAGS_MACBUNDLE = ['-bundle', '-undefined', 'dynamic_lookup']
LINKFLAGS_cshlib = ['-shared']
LINKFLAGS_cstlib = ['-Wl,-Bstatic']
LINKFLAGS_cxxshlib = ['-shared']
LINKFLAGS_cxxstlib = ['-Wl,-Bstatic']
LINK_CC = ['/usr/bin/gcc']
LINK_CXX = ['/usr/bin/g++']
LV2DIR = '/usr/local/lib/lv2'
MANDIR = '/usr/local/share/man'
PARDEBUG = False
PKGCONFIG = '/usr/bin/pkg-config'
PKG_jack = 'jack'
PKG_lilv_0 = 'lilv-0'
PKG_lv2 = 'lv2'
PKG_serd_0 = 'serd-0'
PKG_sord_0 = 'sord-0'
PKG_sratom_0 = 'sratom-0'
PKG_suil_0 = 'suil-0'
PREFIX = '/usr/local'
RPATH_ST = '-Wl,-rpath,%s'
SHLIB_MARKER = '-Wl,-Bdynamic'
SONAME_ST = '-Wl,-h,%s'
STLIBPATH_ST = '-L%s'
STLIB_MARKER = '-Wl,-Bstatic'
STLIB_ST = '-l%s'
SYSCONFDIR = '/usr/local/etc'
VERSION_jack = '0.120.0'
VERSION_lilv-0 = '0.19.2'
VERSION_lv2 = '1.8.1'
VERSION_serd-0 = '0.14.0'
VERSION_sord-0 = '0.12.0'
VERSION_sratom-0 = '0.4.0'
VERSION_suil-0 = '0.6.0'
cfg_files = ['/home/pepper/LV2_render/build/jalv_config.h']
cprogram_PATTERN = '%s'
cshlib_PATTERN = 'lib%s.so'
cstlib_PATTERN = 'lib%s.a'
cxxprogram_PATTERN = '%s'
cxxshlib_PATTERN = 'lib%s.so'
cxxstlib_PATTERN = 'lib%s.a'
define_key = ['HAVE_LV2', 'HAVE_LILV', 'HAVE_SERD', 'HAVE_SORD', 'HAVE_SUIL', 'HAVE_SRATOM', 'HAVE_JACK', 'HAVE_JACK_PORT_TYPE_GET_BUFFER_SIZE', 'HAVE_JACK_METADATA', 'JALV_JACK_SESSION', 'JALV_VERSION']
macbundle_PATTERN = '%s.bundle'
