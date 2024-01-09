#!/bin/bash

# Test if we are root
if [ "$(id -u)" != "0" ]; then
    echo -e "\033[1;31mThis script must be run as root.\033[0m"
    exit 1
fi

# Function to install packages using apt (for Ubuntu and Debian)
install_apt_packages() {
    apt update
    apt install -y libgomp1 libssl3 libjemalloc2 libreadline8 libuuid1
}

# Function to install packages using yum or dnf (for CentOS and RHEL)
install_rhel_packages() {
    \$1 update
    \$1 install -y libgomp openssl jemalloc readline libuuid
    # Note: The versions of these libraries may differ from those in Debian/Ubuntu.
}

# Function to install packages using pacman (for Arch Linux)
install_pacman_packages() {
    pacman -Syu --needed gcc-libs openssl jemalloc readline util-linux
    # Note: gcc-libs provides libgomp, and util-linux provides libuuid.
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
