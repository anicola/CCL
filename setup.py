#!/usr/bin/env python
from setuptools.command.build_py import build_py as _build
from setuptools import setup
from subprocess import call
from io import open
import os
import sys


class build(_build):
    """Specialized Python source builder."""
    def run(self):
        call(["mkdir", "-p", "build"])
        v = sys.version_info
        if call(["cmake", "-H.", "-Bbuild",
                 "-DPYTHON_VERSION=%d.%d.%d" % (
                    v.major, v.minor, v.micro)]) != 0:
            raise Exception(
                "Could not run CMake configuration. Make sure "
                "CMake is installed !")

        if call(["make", "-Cbuild", "_ccllib"]) != 0:
            raise Exception("Could not build CCL")

        # Finds the library under its different possible names
        if os.path.exists("build/pyccl/_ccllib.so"):
            call(["cp", "build/pyccl/_ccllib.so", "pyccl/"])
        else:
            raise Exception("Could not find wrapper shared library, "
                            "compilation must have failed.")
        if call(["cp", "build/pyccl/ccllib.py", "pyccl/"]) != 0:
            raise Exception("Could not find python module, "
                            "SWIG must have failed.")

        # Copy files required by CLASS
        if call(["cp", "-r", "build/extern/share/class/bbn", "pyccl/"]) != 0:
            raise Exception("Could not copy the CLASS BBN fikes, "
                            "CLASS compilation must have failed.")
        if call(["cp", "-r", "build/extern/share/class/hyrec", "pyccl/"]) != 0:
            raise Exception("Could not copy the CLASS BBN fikes, CLASS "
                            "compilation must have failed.")

        _build.run(self)


# read the contents of the README file
with open('README.md', encoding="utf-8") as f:
    long_description = f.read()

setup(
    name="pyccl",
    description="Library of validated cosmological functions.",
    long_description=long_description,
    long_description_content_type='text/markdown',
    author="LSST DESC",
    url="https://github.com/LSSTDESC/CCL",
    packages=['pyccl'],
    provides=['pyccl'],
    package_data={'pyccl': ['_ccllib.so', 'bbn/*', 'hyrec/*']},
    include_package_data=True,
    use_scm_version=True,
    setup_requires=['setuptools_scm'],
    install_requires=['numpy', 'pyyaml'],
    test_suite='nose.collector',
    tests_require=['nose'],
    cmdclass={'build_py': build},
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Topic :: Scientific/Engineering :: Physics'
      ]
    )
