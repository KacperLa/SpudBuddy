from setuptools import setup

setup(
    name="Ron Server",
    version= '1.0',
    long_description='Flask server that serves the api used by Ron.',
    author='Kacper Laska',
    author_email='klaskak@gmail.com',
    include_package_data=True,
    zip_safe=False,
    install_requires=[
        'Flask==2.1.3',
        'pyzmq',
        'numpy',
        'flask-socketio',
        'simple-websocket',
        'opencv-python',
        'posix_ipc'
        ]
)