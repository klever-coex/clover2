from setuptools import find_packages
from setuptools import setup

setup(
    name='clover2_aruco_msgs',
    version='0.0.0',
    packages=find_packages(
        include=('clover2_aruco_msgs', 'clover2_aruco_msgs.*')),
)
