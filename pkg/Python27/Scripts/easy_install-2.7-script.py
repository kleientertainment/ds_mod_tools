#!python.exe
# EASY-INSTALL-ENTRY-SCRIPT: 'setuptools==0.6c11','console_scripts','easy_install-2.7'
__requires__ = 'setuptools==0.6c11'
import sys
from pkg_resources import load_entry_point

sys.exit(
   load_entry_point('setuptools==0.6c11', 'console_scripts', 'easy_install-2.7')()
)
