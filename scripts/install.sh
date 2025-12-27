#!/bin/bash
#
# -- install.sh  - GLE installation script for Linux/macOS
#
# usage
#  to install gle and create filetype association
#   sudo install.sh --prefix=/usr/local
#  to install gle and DO NOT create filetype association
#   sudo install.sh --prefix=/usr/local --no_create_filetype_association
# after installation these can be run
#  to create filetype association
#   sudo install.sh --create_filetype_association
#  to uninstall
#   sudo install.sh --uninstall
#  to uninstall and remove user config files
#   sudo install.sh --uninstall --remove_config

DEFAULT_INSTALL_PREFIX="/usr/local"
echo -e "GLE installation script"
USER_HOME=$(eval echo "~$SUDO_USER")
INSTALL=true
CREATE_ASSOCIATION=true
INSTALL_PREFIX=
UNINSTALL=false
REMOVE_CONFIG=false
APP_NAME="qgle"
DESKTOP_FILE="$USER_HOME/.local/share/applications/qgle.desktop"
MIME_TYPE="text/x-gle-graphics"
MIME_XML="$USER_HOME/.local/share/mime/packages/gle.xml"


# Parse command-line options
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
        --uninstall)
            UNINSTALL=true
            INSTALL=false
            CREATE_ASSOCIATION=false
            ;;
        --create_filetype_association)
            CREATE_ASSOCIATION=true
            INSTALL=false
            ;;
        --no_create_filetype_association)
            CREATE_ASSOCIATION=false
            ;;
        --remove_config)
            REMOVE_CONFIG=true
            INSTALL=false
            ;;
        *)
            echo "Usage: $0 [--prefix /usr|/usr/local] [--uninstall] [--create_filetype_association] [--remove_config]"
            exit 1
            ;;
    esac
    shift
done

if [[ "$INSTALL" == "true" || "$UNINSTALL" == "true" ]]; then
    # Check for root privileges first
    if [[ $EUID -ne 0 ]]; then
        echo "Error: Please run this script as root (e.g., using sudo)."
        exit 1
    fi
fi

if [[ "$INSTALL" == "false" ]]; then
    # not installing - determine if is installed and exit if it is not
    FILE_NAME="gle"
    DIRS=("/usr/local/bin" "/usr/bin")
    INSTALL_PREFIX=""
    for dir in "${DIRS[@]}"; do
        if [[ -f "$dir/$FILE_NAME" ]]; then
            INSTALL_PREFIX="${dir%/bin}"
            break
        fi
    done
    if [[ -z "$INSTALL_PREFIX" ]]; then
        echo "GLE not found in ${DIRS[*]}.  GLE does not appear to be installed.  Exiting."
        exit 1
    fi
fi

if $UNINSTALL; then
    echo "Uninstalling GLE from $INSTALL_PREFIX..."
    rm "$INSTALL_PREFIX/bin/gle" "$INSTALL_PREFIX/bin/qgle" "$INSTALL_PREFIX/bin/manip"
    rm -rf "$INSTALL_PREFIX/share/gle-graphics"
    rm -rf "$INSTALL_PREFIX/share/doc/gle-graphics"
    rm $DESKTOP_FILE
    rm $MIME_XML
    sed -i "/^$MIME_TYPE=/d" "$USER_HOME/.config/mimeapps.list"
    xdg-mime default none $MIME_TYPE
    update-desktop-database ~/.local/share/applications/
    if $REMOVE_CONFIG; then
        echo "Removing GLE config files"
        rm "$USER_HOME/.config/gle"
        rm "$USER_HOME/.glerc"
    fi
    echo "GLE has been removed."
    exit 0
fi

if $INSTALL; then
    # Prompt user for installation directory if no option provided
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
fi

if $CREATE_ASSOCIATION; then
    # does not require root privileges - but might be run as sudo
    SUDO_CMD=
    if [[ -n "$SUDO_USER" ]]; then
        SUDO_CMD="sudo -u $SUDO_USER"
    fi
    EXEC_PATH="$INSTALL_PREFIX/bin/qgle"
    echo "Setting up MIME type and file association for .gle files..."
    # 1. Register MIME type for .gle files
    $SUDO_CMD mkdir -p "$(dirname "$MIME_XML")"
    $SUDO_CMD  cat > "$MIME_XML" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="$MIME_TYPE">
    <comment>GLE Graphics Layout Engine file</comment>
    <glob pattern="*.gle"/>
  </mime-type>
</mime-info>
EOF
    echo "Created MIME definition at $MIME_XML"

    # 2. Update MIME database
    if command -v update-mime-database >/dev/null 2>&1; then
        $SUDO_CMD update-mime-database "$USER_HOME/.local/share/mime"
        echo "Updated MIME database."
    else
        echo "update-mime-database not found. Please install 'shared-mime-info' or reboot to apply changes."
    fi

    # 3. Create .desktop file for QGLE
    $SUDO_CMD mkdir -p "$(dirname "$DESKTOP_FILE")"
    $SUDO_CMD cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Name=$APP_NAME
Exec=$EXEC_PATH %f
Type=Application
MimeType=$MIME_TYPE;
EOF
    echo "Created $DESKTOP_FILE"

    # 4. Update mimeapps.list
    # "$USER_HOME/.local/share/applications/mimeapps.list"
    for FILE in "$USER_HOME/.config/mimeapps.list" ; do
        $SUDO_CMD mkdir -p "$(dirname "$FILE")"
        if ! grep -q "$MIME_TYPE" "$FILE" 2>/dev/null; then
            echo "[Default Applications]" >> "$FILE"
            echo "$MIME_TYPE=qgle.desktop" >> "$FILE"
            echo "Updated $FILE"
        fi
    done

    # # 5. KDE-specific association
    # KDE_FILE="$USER_HOME/.config/kde-mimeapps.list"
    # mkdir -p "$(dirname "$KDE_FILE")"
    # if ! grep -q "$MIME_TYPE" "$KDE_FILE" 2>/dev/null; then
    #     echo "[Added Associations]" >> "$KDE_FILE"
    #     echo "$MIME_TYPE=qgle.desktop;" >> "$KDE_FILE"
    #     echo "Updated $KDE_FILE"
    # fi

    # # 6. Refresh KDE cache
    # if command -v kbuildsycoca6 >/dev/null 2>&1; then
    #     kbuildsycoca6
    # elif command -v kbuildsycoca5 >/dev/null 2>&1; then
    #     kbuildsycoca5
    # fi

    echo "filetype association complete! Log out and back in if changes don't apply immediately."
fi
echo "GLE Installation complete."