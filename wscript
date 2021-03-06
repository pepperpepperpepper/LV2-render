#!/usr/bin/env python
import subprocess
from waflib.extras import autowaf as autowaf
import waflib.Options as Options

# Version of this package (even if built as a child)
#this one? loo ks so
JALV_VERSION = '1.4.6'

# Variables for 'waf dist'
APPNAME = 'LV2-render'
VERSION = JALV_VERSION

# Mandatory variables
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    autowaf.set_options(opt)
    opt.add_option('--no-qt', action='store_true', default=False,
                   dest='no_qt',
                   help="Do not build Qt GUI")

def configure(conf):
    conf.line_just = 52
    conf.load('compiler_c')
    conf.load('compiler_cxx')
    conf.env.append_unique('LIB', 'm') # should be it, not sure so this is some python-based autotools thing, I guess? yeah
    conf.env.append_unique('LIB', 'sndfile') # should be it, not sure so this is some python-based autotools thing, I guess? yeah
    autowaf.configure(conf)
    autowaf.set_c99_mode(conf)
    autowaf.display_header('Jalv Configuration')

    autowaf.check_pkg(conf, 'lv2', atleast_version='1.8.1', uselib_store='LV2')
    autowaf.check_pkg(conf, 'lilv-0', uselib_store='LILV',
                      atleast_version='0.19.2', mandatory=True)
    autowaf.check_pkg(conf, 'serd-0', uselib_store='SERD',
                      atleast_version='0.14.0', mandatory=True)
    autowaf.check_pkg(conf, 'sord-0', uselib_store='SORD',
                      atleast_version='0.12.0', mandatory=True)
    autowaf.check_pkg(conf, 'suil-0', uselib_store='SUIL',
                      atleast_version='0.6.0', mandatory=True)
    autowaf.check_pkg(conf, 'sratom-0', uselib_store='SRATOM',
                      atleast_version='0.4.0', mandatory=True)

    autowaf.define(conf, 'JALV_VERSION', JALV_VERSION)

    conf.write_config_header('jalv_config.h', remove=False)

    print('')

def build(bld):
    libs = 'LILV SUIL SERD SORD SRATOM LV2' 

    source = 'src/LV2-render.c src/symap.c src/state.c src/lv2_evbuf.c src/worker.c src/log.c' 
    source += ' src/midi/midi_loader.c src/midi/fluid_midi.c src/midi/fluid_list.c'

    obj = bld(features     = 'c cprogram',
              source       = source + ' src/LV2-render_console.c',
              target       = 'LV2-render',
              includes     = ['.', 'src', 'midi'],
              lib          = ['pthread'],
              install_path = '${BINDIR}')
    autowaf.use_lib(bld, obj, libs)


def lint(ctx):
    subprocess.call('cpplint.py --filter=+whitespace/comments,-whitespace/tab,-whitespace/braces,-whitespace/labels,-build/header_guard,-readability/casting,-readability/todo,-build/include,-runtime/sizeof src/* jalv/*', shell=True)
