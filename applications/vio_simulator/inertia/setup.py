#!/usr/bin/env python
from distutils.core import setup

setup(name='inertia',
      description='IMU simulator and integrator',
      version='0.1',
      author='Alex Flint',
      author_email='alex.flint@gmail.com',
      url='https://github.com/alexflint/inertia',
      packages=['inertia', 'inertia.simulator'],
      package_dir={'inertia': '.'},
      scripts=['simulator/cmds/simulate_visual_inertial']
      )
