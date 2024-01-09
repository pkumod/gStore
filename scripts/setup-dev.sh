#!/bin/bash

# Test if we are root
if [ "$(id -u)" != "0" ]; then
    echo -e "\033[1;31mThis script must be run as root.\033[0m"
    exit 1
fi

# Function to install packages using apt
install_apt_packages() {
    apt-get update
    apt-get install -y build-essential cmake ninja-build mold python3-pip \
        pkg-config uuid-dev libjemalloc-dev libreadline-dev libssl-dev lcov
}

# Function to install packages using yum
install_rhel_packages() {
    \$1 update
    \$1 install -y @'Development Tools' cmake ninja-build python3-pip \
        pkgconfig uuid-devel jemalloc-devel readline-devel openssl-devel lcov
    echo -e "\033[1;33mNote: Mold is not available in the official repositories of CentOS and RHEL.\033[0m"
    echo -e "\033[1;33m      You can install it manually from https://github.com/rui314/mold/releases.\033[0m"
}

# Function to install packages using pacman
install_pacman_packages() {
    pacman -Syu --needed base-devel cmake ninja mold python-pip \
        pkgconf util-linux jemalloc readline openssl lcov
    # Note: In Arch Linux, 'base-devel' group includes tools similar to 'build-essential'
}

# Check for the presence of each package manager and install packages accordingly
if command -v apt &> /dev/null; then
    install_apt_packages
elif command -v dnf &> /dev/null; then
    install_rhel_packages dnf
elif command -v yum &> /dev/null; then
    install_rhel_packages yum
elif command -v pacman &> /dev/null; then
    install_pacman_packages
else
    echo "No known package manager found. Are you sure this is a supported Linux distribution?"
    exit 1
fi

echo "Setting limits..."
echo "*    -    nofile    65535" | tee -a /etc/security/limits.conf
echo "*    -    noproc    65535" | tee -a /etc/security/limits.conf

echo "Installation completed."