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
        'flask[async]',
        'numpy==1.19.3',
        'flask-socketio',
        'simple-websocket',
        'opencv-python',
        'PyTurboJPEG',
        'aiortc',
        'aiohttp',
        ]
)