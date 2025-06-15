echo "I really hope this works!"

# This is for ubuntu systems
if [  -n "$(uname -a | grep Ubuntu)" ]; then
    sudo apt-get update && sudo apt-get upgrade
    sudo apt-get install libwayland-dev libxkbcommon-dev libfreetype-dev libglfw3-dev libgles2-mesa-dev
    make -b 
fi

echo "Done installing and building pelase work pelase please please" 
