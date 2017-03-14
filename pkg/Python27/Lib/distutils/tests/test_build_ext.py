import sys
import os
import tempfile
import shutil
from StringIO import StringIO
import textwrap

from distutils.core import Extension, Distribution
from distutils.command.build_ext import build_ext
from distutils import sysconfig
from distutils.tests import support
from distutils.errors import DistutilsSetupError, CompileError

import unittest
from test import test_support

# http://bugs.python.org/issue4373
# Don't load the xx module more than once.
ALREADY_TESTED = False

def _get_source_filename():
    srcdir = sysconfig.get_config_var('srcdir')
    if srcdir is None:
        return os.path.join(sysconfig.project_base, 'Modules', 'xxmodule.c')
    return os.path.join(srcdir, 'Modules', 'xxmodule.c')

_XX_MODULE_PATH = _get_source_filename()

class BuildExtTestCase(support.TempdirManager,
                       support.LoggingSilencer,
                       unittest.TestCase):
    def setUp(self):
        # Create a simple test environment
        # Note that we're making changes to sys.path
        super(BuildExtTestCase, self).setUp()
        self.tmp_dir = tempfile.mkdtemp(prefix="pythontest_")
        if os.path.exists(_XX_MODULE_PATH):
            self.sys_path = sys.path[:]
            sys.path.append(self.tmp_dir)
            shutil.copy(_XX_MODULE_PATH, self.tmp_dir)

    def tearDown(self):
        # Get everything back to normal
        if os.path.exists(_XX_MODULE_PATH):
            test_support.unload('xx')
            sys.path[:] = self.sys_path
            # XXX on Windows the test leaves a directory
            # with xx module in TEMP
        shutil.rmtree(self.tmp_dir, os.name == 'nt' or
                                    sys.platform == 'cygwin')
        super(BuildExtTestCase, self).tearDown()

    def _fixup_command(self, cmd):
        # When Python was build with --enable-shared, -L. is not good enough
        # to find the libpython<blah>.so.  This is because regrtest runs it
        # under a tempdir, not in the top level where the .so lives.  By the
        # time we've gotten here, Python's already been chdir'd to the
        # tempdir.
        #
        # To further add to the fun, we can't just add library_dirs to the
        # Extension() instance because that doesn't get plumbed through to the
        # final compiler command.
        if (sysconfig.get_config_var('Py_ENABLE_SHARED') and
            not sys.platform.startswith('win')):
            runshared = sysconfig.get_config_var('RUNSHARED')
            if runshared is None:
                cmd.library_dirs = ['.']
            else:
                name, equals, value = runshared.partition('=')
                cmd.library_dirs = value.split(os.pathsep)

    @unittest.skipIf(not os.path.exists(_XX_MODULE_PATH),
                     'xxmodule.c not found')
    def test_build_ext(self):
        global ALREADY_TESTED
        xx_c = os.path.join(self.tmp_dir, 'xxmodule.c')
        xx_ext = Extension('xx', [xx_c])
        dist = Distribution({'name': 'xx', 'ext_modules': [xx_ext]})
        dist.package_dir = self.tmp_dir
        cmd = build_ext(dist)
        self._fixup_command(cmd)
        if os.name == "nt":
            # On Windows, we must build a debug version iff running
            # a debug build of Python
            cmd.debug = sys.executable.endswith("_d.exe")
        cmd.build_lib = self.tmp_dir
        cmd.build_temp = self.tmp_dir

        old_stdout = sys.stdout
        if not test_support.verbose:
            # silence compiler output
            sys.stdout = StringIO()
        try:
            cmd.ensure_finalized()
            cmd.run()
        finally:
            sys.stdout = old_stdout

        if ALREADY_TESTED:
            return
        else:
            ALREADY_TESTED = True

        import xx

        for attr in ('error', 'foo', 'new', 'roj'):
            self.assertTrue(hasattr(xx, attr))

        self.assertEqual(xx.foo(2, 5), 7)
        self.assertEqual(xx.foo(13,15), 28)
        self.assertEqual(xx.new().demo(), None)
        doc = 'This is a template module just for instruction.'
        self.assertEqual(xx.__doc__, doc)
        self.assertTrue(isinstance(xx.Null(), xx.Null))
        self.assertTrue(isinstance(xx.Str(), xx.Str))

    def test_solaris_enable_shared(self):
        dist = Distribution({'name': 'xx'})
        cmd = build_ext(dist)
        old = sys.platform

        sys.platform = 'sunos' # fooling finalize_options
        from distutils.sysconfig import  _config_vars
        old_var = _config_vars.get('Py_ENABLE_SHARED')
        _config_vars['Py_ENABLE_SHARED'] = 1
        try:
            cmd.ensure_finalized()
        finally:
            sys.platform = old
            if old_var is None:
                del _config_vars['Py_ENABLE_SHARED']
            else:
                _config_vars['Py_ENABLE_SHARED'] = old_var

        # make sure we get some library dirs under solaris
        self.assertTrue(len(cmd.library_dirs) > 0)

    def test_finalize_options(self):
        # Make sure Python's include directories (for Python.h, pyconfig.h,
        # etc.) are in the include search path.
        modules = [Extension('foo', ['xxx'])]
        dist = Distribution({'name': 'xx', 'ext_modules': modules})
        cmd = build_ext(dist)
        cmd.finalize_options()

        from distutils import sysconfig
        py_include = sysconfig.get_python_inc()
        self.assertTrue(py_include in cmd.include_dirs)

        plat_py_include = sysconfig.get_python_inc(plat_specific=1)
        self.assertTrue(plat_py_include in cmd.include_dirs)

        # make sure cmd.libraries is turned into a list
        # if it's a string
        cmd = build_ext(dist)
        cmd.libraries = 'my_lib'
        cmd.finalize_options()
        self.assertEqual(cmd.libraries, ['my_lib'])

        # make sure cmd.library_dirs is turned into a list
        # if it's a string
        cmd = build_ext(dist)
        cmd.library_dirs = 'my_lib_dir'
        cmd.finalize_options()
        self.assertTrue('my_lib_dir' in cmd.library_dirs)

        # make sure rpath is turned into a list
        # if it's a list of os.pathsep's paths
        cmd = build_ext(dist)
        cmd.rpath = os.pathsep.join(['one', 'two'])
        cmd.finalize_options()
        self.assertEqual(cmd.rpath, ['one', 'two'])

        # XXX more tests to perform for win32

        # make sure define is turned into 2-tuples
        # strings if they are ','-separated strings
        cmd = build_ext(dist)
        cmd.define = 'one,two'
        cmd.finalize_options()
        self.assertEqual(cmd.define, [('one', '1'), ('two', '1')])

        # make sure undef is turned into a list of
        # strings if they are ','-separated strings
        cmd = build_ext(dist)
        cmd.undef = 'one,two'
        cmd.finalize_options()
        self.assertEqual(cmd.undef, ['one', 'two'])

        # make sure swig_opts is turned into a list
        cmd = build_ext(dist)
        cmd.swig_opts = None
        cmd.finalize_options()
        self.assertEqual(cmd.swig_opts, [])

        cmd = build_ext(dist)
        cmd.swig_opts = '1 2'
        cmd.finalize_options()
        self.assertEqual(cmd.swig_opts, ['1', '2'])

    def test_check_extensions_list(self):
        dist = Distribution()
        cmd = build_ext(dist)
        cmd.finalize_options()

        #'extensions' option must be a list of Extension instances
        self.assertRaises(DistutilsSetupError, cmd.check_extensions_list, 'foo')

        # each element of 'ext_modules' option must be an
        # Extension instance or 2-tuple
        exts = [('bar', 'foo', 'bar'), 'foo']
        self.assertRaises(DistutilsSetupError, cmd.check_extensions_list, exts)

        # first element of each tuple in 'ext_modules'
        # must be the extension name (a string) and match
        # a python dotted-separated name
        exts = [('foo-bar', '')]
        self.assertRaises(DistutilsSetupError, cmd.check_extensions_list, exts)

        # second element of each tuple in 'ext_modules'
        # must be a ary (build info)
        exts = [('foo.bar', '')]
        self.assertRaises(DistutilsSetupError, cmd.check_extensions_list, exts)

        # ok this one should pass
        exts = [('foo.bar', {'sources': [''], 'libraries': 'foo',
                             'some': 'bar'})]
        cmd.check_extensions_list(exts)
        ext = exts[0]
        self.assertTrue(isinstance(ext, Extension))

        # check_extensions_list adds in ext the values passed
        # when they are in ('include_dirs', 'library_dirs', 'libraries'
        # 'extra_objects', 'extra_compile_args', 'extra_link_args')
        self.assertEqual(ext.libraries, 'foo')
        self.assertTrue(not hasattr(ext, 'some'))

        # 'macros' element of build info dict must be 1- or 2-tuple
        exts = [('foo.bar', {'sources': [''], 'libraries': 'foo',
                'some': 'bar', 'macros': [('1', '2', '3'), 'foo']})]
        self.assertRaises(DistutilsSetupError, cmd.check_extensions_list, exts)

        exts[0][1]['macros'] = [('1', '2'), ('3',)]
        cmd.check_extensions_list(exts)
        self.assertEqual(exts[0].undef_macros, ['3'])
        self.assertEqual(exts[0].define_macros, [('1', '2')])

    def test_get_source_files(self):
        modules = [Extension('foo', ['xxx'])]
        dist = Distribution({'name': 'xx', 'ext_modules': modules})
        cmd = build_ext(dist)
        cmd.ensure_finalized()
        self.assertEqual(cmd.get_source_files(), ['xxx'])

    def test_compiler_option(self):
        # cmd.compiler is an option and
        # should not be overriden by a compiler instance
        # when the command is run
        dist = Distribution()
        cmd = build_ext(dist)
        cmd.compiler = 'unix'
        cmd.ensure_finalized()
        cmd.run()
        self.assertEqual(cmd.compiler, 'unix')

    def test_get_outputs(self):
        tmp_dir = self.mkdtemp()
        c_file = os.path.join(tmp_dir, 'foo.c')
        self.write_file(c_file, 'void initfoo(void) {};\n')
        ext = Extension('foo', [c_file])
        dist = Distribution({'name': 'xx',
                             'ext_modules': [ext]})
        cmd = build_ext(dist)
        self._fixup_command(cmd)
        cmd.ensure_finalized()
        self.assertEqual(len(cmd.get_outputs()), 1)

        if os.name == "nt":
            cmd.debug = sys.executable.endswith("_d.exe")

        cmd.build_lib = os.path.join(self.tmp_dir, 'build')
        cmd.build_temp = os.path.join(self.tmp_dir, 'tempt')

        # issue #5977 : distutils build_ext.get_outputs
        # returns wrong result with --inplace
        other_tmp_dir = os.path.realpath(self.mkdtemp())
        old_wd = os.getcwd()
        os.chdir(other_tmp_dir)
        try:
            cmd.inplace = 1
            cmd.run()
            so_file = cmd.get_outputs()[0]
        finally:
            os.chdir(old_wd)
        self.assertTrue(os.path.exists(so_file))
        self.assertEqual(os.path.splitext(so_file)[-1],
                         sysconfig.get_config_var('SO'))
        so_dir = os.path.dirname(so_file)
        self.assertEqual(so_dir, other_tmp_dir)
        cmd.compiler = None
        cmd.inplace = 0
        cmd.run()
        so_file = cmd.get_outputs()[0]
        self.assertTrue(os.path.exists(so_file))
        self.assertEqual(os.path.splitext(so_file)[-1],
                         sysconfig.get_config_var('SO'))
        so_dir = os.path.dirname(so_file)
        self.assertEqual(so_dir, cmd.build_lib)

        # inplace = 0, cmd.package = 'bar'
        build_py = cmd.get_finalized_command('build_py')
        build_py.package_dir = {'': 'bar'}
        path = cmd.get_ext_fullpath('foo')
        # checking that the last directory is the build_dir
        path = os.path.split(path)[0]
        self.assertEqual(path, cmd.build_lib)

        # inplace = 1, cmd.package = 'bar'
        cmd.inplace = 1
        other_tmp_dir = os.path.realpath(self.mkdtemp())
        old_wd = os.getcwd()
        os.chdir(other_tmp_dir)
        try:
            path = cmd.get_ext_fullpath('foo')
        finally:
            os.chdir(old_wd)
        # checking that the last directory is bar
        path = os.path.split(path)[0]
        lastdir = os.path.split(path)[-1]
        self.assertEqual(lastdir, 'bar')

    def test_ext_fullpath(self):
        ext = sysconfig.get_config_vars()['SO']
        dist = Distribution()
        cmd = build_ext(dist)
        cmd.inplace = 1
        cmd.distribution.package_dir = {'': 'src'}
        cmd.distribution.packages = ['lxml', 'lxml.html']
        curdir = os.getcwd()
        wanted = os.path.join(curdir, 'src', 'lxml', 'etree' + ext)
        path = cmd.get_ext_fullpath('lxml.etree')
        self.assertEqual(wanted, path)

        # building lxml.etree not inplace
        cmd.inplace = 0
        cmd.build_lib = os.path.join(curdir, 'tmpdir')
        wanted = os.path.join(curdir, 'tmpdir', 'lxml', 'etree' + ext)
        path = cmd.get_ext_fullpath('lxml.etree')
        self.assertEqual(wanted, path)

        # building twisted.runner.portmap not inplace
        build_py = cmd.get_finalized_command('build_py')
        build_py.package_dir = {}
        cmd.distribution.packages = ['twisted', 'twisted.runner.portmap']
        path = cmd.get_ext_fullpath('twisted.runner.portmap')
        wanted = os.path.join(curdir, 'tmpdir', 'twisted', 'runner',
                              'portmap' + ext)
        self.assertEqual(wanted, path)

        # building twisted.runner.portmap inplace
        cmd.inplace = 1
        path = cmd.get_ext_fullpath('twisted.runner.portmap')
        wanted = os.path.join(curdir, 'twisted', 'runner', 'portmap' + ext)
        self.assertEqual(wanted, path)

    def test_build_ext_inplace(self):
        etree_c = os.path.join(self.tmp_dir, 'lxml.etree.c')
        etree_ext = Extension('lxml.etree', [etree_c])
        dist = Distribution({'name': 'lxml', 'ext_modules': [etree_ext]})
        cmd = build_ext(dist)
        cmd.ensure_finalized()
        cmd.inplace = 1
        cmd.distribution.package_dir = {'': 'src'}
        cmd.distribution.packages = ['lxml', 'lxml.html']
        curdir = os.getcwd()
        ext = sysconfig.get_config_var("SO")
        wanted = os.path.join(curdir, 'src', 'lxml', 'etree' + ext)
        path = cmd.get_ext_fullpath('lxml.etree')
        self.assertEqual(wanted, path)

    def test_setuptools_compat(self):
        import distutils.core, distutils.extension, distutils.command.build_ext
        saved_ext = distutils.extension.Extension
        try:
            # on some platforms, it loads the deprecated "dl" module
            test_support.import_module('setuptools_build_ext', deprecated=True)

            # theses import patch Distutils' Extension class
            from setuptools_build_ext import build_ext as setuptools_build_ext
            from setuptools_extension import Extension

            etree_c = os.path.join(self.tmp_dir, 'lxml.etree.c')
            etree_ext = Extension('lxml.etree', [etree_c])
            dist = Distribution({'name': 'lxml', 'ext_modules': [etree_ext]})
            cmd = setuptools_build_ext(dist)
            cmd.ensure_finalized()
            cmd.inplace = 1
            cmd.distribution.package_dir = {'': 'src'}
            cmd.distribution.packages = ['lxml', 'lxml.html']
            curdir = os.getcwd()
            ext = sysconfig.get_config_var("SO")
            wanted = os.path.join(curdir, 'src', 'lxml', 'etree' + ext)
            path = cmd.get_ext_fullpath('lxml.etree')
            self.assertEqual(wanted, path)
        finally:
            # restoring Distutils' Extension class otherwise its broken
            distutils.extension.Extension = saved_ext
            distutils.core.Extension = saved_ext
            distutils.command.build_ext.Extension = saved_ext

    def test_build_ext_path_with_os_sep(self):
        dist = Distribution({'name': 'UpdateManager'})
        cmd = build_ext(dist)
        cmd.ensure_finalized()
        ext = sysconfig.get_config_var("SO")
        ext_name = os.path.join('UpdateManager', 'fdsend')
        ext_path = cmd.get_ext_fullpath(ext_name)
        wanted = os.path.join(cmd.build_lib, 'UpdateManager', 'fdsend' + ext)
        self.assertEqual(ext_path, wanted)

    def test_build_ext_path_cross_platform(self):
        if sys.platform != 'win32':
            return
        dist = Distribution({'name': 'UpdateManager'})
        cmd = build_ext(dist)
        cmd.ensure_finalized()
        ext = sysconfig.get_config_var("SO")
        # this needs to work even under win32
        ext_name = 'UpdateManager/fdsend'
        ext_path = cmd.get_ext_fullpath(ext_name)
        wanted = os.path.join(cmd.build_lib, 'UpdateManager', 'fdsend' + ext)
        self.assertEqual(ext_path, wanted)

    @unittest.skipUnless(sys.platform == 'darwin', 'test only relevant for MacOSX')
    def test_deployment_target(self):
        self._try_compile_deployment_target()

        orig_environ = os.environ
        os.environ = orig_environ.copy()
        self.addCleanup(setattr, os, 'environ', orig_environ)

        os.environ['MACOSX_DEPLOYMENT_TARGET']='10.1'
        self._try_compile_deployment_target()


    def _try_compile_deployment_target(self):
        deptarget_c = os.path.join(self.tmp_dir, 'deptargetmodule.c')

        with open(deptarget_c, 'w') as fp:
            fp.write(textwrap.dedent('''\
                #include <AvailabilityMacros.h>

                int dummy;

                #if TARGET != MAC_OS_X_VERSION_MIN_REQUIRED
                #error "Unexpected target"
               #endif

            '''))

        target = sysconfig.get_config_var('MACOSX_DEPLOYMENT_TARGET')
        target = tuple(map(int, target.split('.')))
        target = '%02d%01d0' % target

        deptarget_ext = Extension(
            'deptarget',
            [deptarget_c],
            extra_compile_args=['-DTARGET=%s'%(target,)],
        )
        dist = Distribution({
            'name': 'deptarget',
            'ext_modules': [deptarget_ext]
        })
        dist.package_dir = self.tmp_dir
        cmd = build_ext(dist)
        cmd.build_lib = self.tmp_dir
        cmd.build_temp = self.tmp_dir

        try:
            old_stdout = sys.stdout
            cmd.ensure_finalized()
            cmd.run()

        except CompileError:
            self.fail("Wrong deployment target during compilation")

def test_suite():
    return unittest.makeSuite(BuildExtTestCase)

if __name__ == '__main__':
    test_support.run_unittest(test_suite())
