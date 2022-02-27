cd ./kernel_module && make clean
cd ../library && make clean
cd ../benchmark && make clean

cd ../kernel_module && make && sudo make install && sudo insmod blockmma.ko && sudo insmod blockmma.ko && sudo chmod 777 /dev/blockmma
cd ../library && make && sudo make install
cd ../benchmark && make && source ./run.sh

cd ..