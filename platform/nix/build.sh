pushd build

sudo find ../src/ -name \*.cpp -exec gcc -I"../include" -I"../dyncall" -c {} +
sudo find ../platform/nix/src/ -name \*.cpp -exec gcc -I"../include" -I"../dyncall" -c {} +

g++ -o ../bin/light -ldl *.o ../dyncall/dyncall/libdyncall_s.a

popd
