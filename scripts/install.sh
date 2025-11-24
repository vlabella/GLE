#!/bin/bash
#
# -- install.sh  - GLE installation script for Linux/macOS
#
DEFAULT_INSTALL_PREFIX="/usr/local"
echo -e "\e[31mG\e[32mL\e[34mE\e[0m installation script"

# Check for root privileges first
if [[ $EUID -ne 0 ]]; then
    echo "Error: Please run this script as root (e.g., using sudo)."
    exit 1
fi

# Prompt user for installation directory if no option provided

INSTALL_PREFIX=
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)
            shift
            if [[ "$1" == "/usr" || "$1" == "/usr/local" ]]; then
                INSTALL_PREFIX="$1"
            else
                echo "Invalid prefix. Use /usr or /usr/local."
                exit 1
            fi
            ;;
        *)
            echo "Usage: $0 [--prefix /usr|/usr/local]"
            exit 1
            ;;
    esac
    shift
done

# If no prefix provided, prompt user with default /usr/local
if [[ -z "$INSTALL_PREFIX" ]]; then
    read -p "Choose installation directory [$DEFAULT_INSTALL_PREFIX by default]: " USER_INPUT
    if [[ -z "$USER_INPUT" ]]; then
        INSTALL_PREFIX=$DEFAULT_INSTALL_PREFIX
    elif [[ "$USER_INPUT" == "/usr" || "$USER_INPUT" == "/usr/local" ]]; then
        INSTALL_PREFIX="$USER_INPUT"
    else
        echo "Invalid choice. Please run the script again and choose /usr or /usr/local."
        exit 1
    fi
fi

echo "Installing GLE to $INSTALL_PREFIX..."

# Create directories
mkdir -p "$INSTALL_PREFIX/bin"
mkdir -p "$INSTALL_PREFIX/share/doc/gle-graphics"
mkdir -p "$INSTALL_PREFIX/share/gle-graphics"

# Copy files
cp -v bin/gle "$INSTALL_PREFIX/bin/"
cp -v bin/qgle "$INSTALL_PREFIX/bin/"
cp -v bin/manip "$INSTALL_PREFIX/bin/"
cp glerc "$INSTALL_PREFIX/share/gle-graphics"
cp init.tex "$INSTALL_PREFIX/share/gle-graphics"
cp inittex.ini "$INSTALL_PREFIX/share/gle-graphics"
cp -v doc/* "$INSTALL_PREFIX/share/doc/gle-graphics/"
cp -r gleinc "$INSTALL_PREFIX/share/gle-graphics/gleinc"
cp -r font "$INSTALL_PREFIX/share/gle-graphics/font"
# set environment variable
export GLE_TOP=$INSTALL_PREFIX/share/gle-graphics
# run finddeps
sudo -u "$SUDO_USER" gle -finddeps
# show user information
sudo -u "$SUDO_USER" gle -info

echo "GLE Installation complete."
