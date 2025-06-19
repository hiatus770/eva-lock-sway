#!/bin/bash

echo "Eva Lock Sway - Installation Script"
echo "==================================="

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if a package is installed (pkg-config)
check_pkg_config() {
    pkg-config --exists "$1" 2>/dev/null
}

# Function to check if a library exists
check_library() {
    ldconfig -p | grep -q "$1" 2>/dev/null
}

# Detect distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    elif command_exists lsb_release; then
        lsb_release -si | tr '[:upper:]' '[:lower:]'
    elif [ -f /etc/debian_version ]; then
        echo "debian"
    elif [ -f /etc/redhat-release ]; then
        echo "rhel"
    else
        echo "unknown"
    fi
}

# Check required dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    missing_deps=()
    missing_tools=()
    
    # Required development tools
    tools=("gcc" "make" "pkg-config")
    for tool in "${tools[@]}"; do
        if ! command_exists "$tool"; then
            missing_tools+=("$tool")
        fi
    done
    
    # Required libraries (pkg-config names)
    pkg_deps=("wayland-client" "wayland-egl" "egl" "glesv2" "xkbcommon" "wayland-protocols" "freetype2")
    for dep in "${pkg_deps[@]}"; do
        if ! check_pkg_config "$dep"; then
            missing_deps+=("$dep")
        fi
    done
    
    # Check for wayland-scanner specifically
    if ! command_exists "wayland-scanner"; then
        missing_tools+=("wayland-scanner")
    fi
    
    # Optional but recommended: meson and ninja for better build system
    if ! command_exists "meson"; then
        print_warning "Meson build system not found (optional but recommended)"
    fi
    
    if ! command_exists "ninja"; then
        print_warning "Ninja build tool not found (optional but recommended)"
    fi
    
    return 0
}

# Install dependencies based on distribution
install_dependencies() {
    distro=$(detect_distro)
    print_status "Detected distribution: $distro"
    
    case "$distro" in
        "ubuntu"|"debian"|"pop"|"mint")
            print_status "Installing dependencies using apt..."
            sudo apt-get update
            
            # Development tools
            if [[ ${#missing_tools[@]} -gt 0 ]]; then
                sudo apt-get install -y build-essential pkg-config wayland-protocols
            fi
            
            # Libraries
            sudo apt-get install -y \
                libwayland-dev \
                libwayland-egl1-mesa-dev \
                libxkbcommon-dev \
                libfreetype-dev \
                libegl1-mesa-dev \
                libgles2-mesa-dev \
                wayland-protocols \
                libwayland-bin
            ;;
        "fedora"|"rhel"|"centos"|"rocky"|"almalinux")
            print_status "Installing dependencies using dnf/yum..."
            
            if command_exists dnf; then
                PKG_MGR="dnf"
            else
                PKG_MGR="yum"
            fi
            
            sudo $PKG_MGR install -y \
                gcc \
                make \
                pkgconfig \
                wayland-devel \
                wayland-protocols-devel \
                libxkbcommon-devel \
                freetype-devel \
                mesa-libEGL-devel \
                mesa-libGLES-devel
            ;;
        "arch"|"manjaro")
            print_status "Installing dependencies using pacman..."
            
            sudo pacman -Sy --needed \
                gcc \
                make \
                pkgconf \
                wayland \
                wayland-protocols \
                libxkbcommon \
                freetype2 \
                egl-wayland \
                gles
            ;;
        "opensuse"|"suse")
            print_status "Installing dependencies using zypper..."
            
            sudo zypper install -y \
                gcc \
                make \
                pkg-config \
                wayland-devel \
                wayland-protocols-devel \
                libxkbcommon-devel \
                freetype2-devel \
                Mesa-libEGL-devel \
                Mesa-libGLESv2-devel
            ;;
        *)
            print_error "Unsupported distribution: $distro"
            print_status "Please install the following dependencies manually:"
            print_status "Development tools: gcc, make, pkg-config, wayland-scanner"
            print_status "Libraries: wayland-client, wayland-egl, xkbcommon, freetype2, EGL, GLESv2"
            return 1
            ;;
    esac
    
    return 0
}

# Build the project
build_project() {
    print_status "Building the project..."
    
    # Update PROJECT_DIRECTORY in globals.h to current directory
    print_status "Updating PROJECT_DIRECTORY in globals.h..."
    current_dir=$(pwd)
    
    if [ -f "include/globals.h" ]; then
        # Create a backup of the original file
        cp include/globals.h include/globals.h.backup
        
        # Update the PROJECT_DIRECTORY define
        sed -i "s|#define PROJECT_DIRECTORY.*|#define PROJECT_DIRECTORY \"$current_dir\"|" include/globals.h
        
        if [ $? -eq 0 ]; then
            print_success "Updated PROJECT_DIRECTORY to: $current_dir"
        else
            print_warning "Failed to update PROJECT_DIRECTORY, continuing with build..."
        fi
    else
        print_warning "include/globals.h not found, skipping PROJECT_DIRECTORY update"
    fi
    
    # Use make build system
    print_status "Using make build system..."
    if [ -f "makefile" ] || [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
        make build/main
        
        if [ $? -eq 0 ]; then
            print_success "Build completed successfully with make!"
            return 0
        else
            print_error "Build failed!"
            return 1
        fi
    else
        print_error "No makefile found!"
        return 1
    fi
}

# Main installation process
main() {
    print_status "Starting installation process..."
    
    # Check dependencies first
    check_dependencies
    
    # Show missing dependencies
    if [[ ${#missing_deps[@]} -gt 0 ]] || [[ ${#missing_tools[@]} -gt 0 ]]; then
        print_warning "Missing dependencies detected:"
        
        if [[ ${#missing_tools[@]} -gt 0 ]]; then
            print_warning "Missing tools: ${missing_tools[*]}"
        fi
        
        if [[ ${#missing_deps[@]} -gt 0 ]]; then
            print_warning "Missing libraries: ${missing_deps[*]}"
        fi
        
        echo
        read -p "Would you like to install missing dependencies? (y/N): " -n 1 -r
        echo
        
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            install_dependencies
            if [ $? -ne 0 ]; then
                print_error "Failed to install dependencies"
                exit 1
            fi
        else
            print_error "Cannot proceed without required dependencies"
            exit 1
        fi
    else
        print_success "All dependencies are already installed!"
    fi
    
    # Verify dependencies after installation
    print_status "Verifying dependencies..."
    check_dependencies
    
    remaining_missing=()
    for dep in "${missing_deps[@]}"; do
        if ! check_pkg_config "$dep"; then
            remaining_missing+=("$dep")
        fi
    done
    
    for tool in "${missing_tools[@]}"; do
        if ! command_exists "$tool"; then
            remaining_missing+=("$tool")
        fi
    done
    
    if [[ ${#remaining_missing[@]} -gt 0 ]]; then
        print_error "Some dependencies are still missing: ${remaining_missing[*]}"
        print_error "Please install them manually and run this script again"
        exit 1
    fi
    
    print_success "All dependencies verified!"
    
    # Build the project
    build_project
    if [ $? -eq 0 ]; then
        print_success "Installation completed successfully!"
        print_status "You can now run the application from the build directory"
        
        if [ -f "build/waylandplaying" ]; then
            print_status "Meson binary: ./build/waylandplaying"
        fi
        if [ -f "build/main" ]; then
            print_status "Make binary: ./build/main"
        fi
    else
        print_error "Installation failed during build step"
        exit 1
    fi
}

# Run main function
main "$@" 
