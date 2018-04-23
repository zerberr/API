# How to build (x64 using MSVS)

~~~~
git clone https://gitlab.com/credits_bc/temp/csconnector.git
cd csconnector
git submodule update --init --recursive
md build64
cd build64
cmake -G "Visual Studio 14 2015 Win64" ..
~~~~

Find and use generated solution to build with MSVS