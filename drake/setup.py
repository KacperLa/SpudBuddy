from setuptools import setup

setup(
    name="Ron Sim",
    version= '1.0',
    long_description='drake sim that serves the simulation backend to ron Ron with out hardware.',
    author='Kacper Laska',
    author_email='klaskak@gmail.com',
    include_package_data=True,
    zip_safe=False,
    install_requires=[
        'numpy',
        'drake'
        ]
)