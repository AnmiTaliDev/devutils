#!/bin/sh

# configure.sh - Configuration script for dev-utils
# Generated: 2025-03-20 07:34:01 UTC by AnmiTaliDev

# Default values
PREFIX="/usr/local"
BINDIR=""
MANDIR=""
CC="clang"
CFLAGS="-O2 -Wall -Wextra -Wpedantic -std=c11"
LDFLAGS=""
LIBS=""
PACKAGE="dev-utils"
VERSION="1.0.0"
BUILD_TYPE="release"

# Function to display help
show_help() {
    cat << EOF
Usage: ./configure.sh [options]

Configuration options:
  --prefix=DIR          Install files in PREFIX [default: /usr/local]
  --bindir=DIR          User executables [PREFIX/bin]
  --mandir=DIR          Man documentation [PREFIX/share/man]
  --cc=CC              Set C compiler [default: clang]
  
Build options:
  --debug              Build with debug symbols (-g -O0 -DDEBUG)
  --release            Build with optimizations [default]
  --sanitize=TYPE      Enable sanitizer (address, thread, memory, undefined)
  
Advanced options:
  --extra-cflags=FLAGS Additional compiler flags
  --extra-ldflags=FLAGS Additional linker flags
  --libs=LIBS          Additional libraries to link against

Other options:
  --help              Display this help and exit
  --version           Display version information and exit
EOF
    exit 0
}

# Function to display version
show_version() {
    echo "$PACKAGE version $VERSION"
    exit 0
}

# Function to check compiler
check_compiler() {
    if ! command -v "$CC" >/dev/null 2>&1; then
        echo "Error: $CC not found"
        echo "Please install clang or specify another compiler with --cc=<compiler>"
        exit 1
    fi
}

# Parse command line arguments
while [ $# -gt 0 ]; do
    case $1 in
        --help)
            show_help
            ;;
        --version)
            show_version
            ;;
        --prefix=*)
            PREFIX="${1#*=}"
            ;;
        --bindir=*)
            BINDIR="${1#*=}"
            ;;
        --mandir=*)
            MANDIR="${1#*=}"
            ;;
        --cc=*)
            CC="${1#*=}"
            ;;
        --debug)
            BUILD_TYPE="debug"
            CFLAGS="-g -O0 -DDEBUG"
            ;;
        --release)
            BUILD_TYPE="release"
            CFLAGS="-O2 -DNDEBUG"
            ;;
        --sanitize=*)
            SANITIZE="${1#*=}"
            case $SANITIZE in
                address|thread|memory|undefined)
                    CFLAGS="$CFLAGS -fsanitize=$SANITIZE"
                    LDFLAGS="$LDFLAGS -fsanitize=$SANITIZE"
                    ;;
                *)
                    echo "Error: Unknown sanitizer type: $SANITIZE"
                    exit 1
                    ;;
            esac
            ;;
        --extra-cflags=*)
            CFLAGS="$CFLAGS ${1#*=}"
            ;;
        --extra-ldflags=*)
            LDFLAGS="$LDFLAGS ${1#*=}"
            ;;
        --libs=*)
            LIBS="${1#*=}"
            ;;
        *)
            echo "Error: Unknown option: $1"
            echo "Try './configure.sh --help' for more information"
            exit 1
            ;;
    esac
    shift
done

# Set default installation directories if not specified
BINDIR="${BINDIR:-$PREFIX/bin}"
MANDIR="${MANDIR:-$PREFIX/share/man}"

# Check requirements
echo "Checking build requirements..."
check_compiler

# Create config.h
echo "Creating config.h..."
cat > config.h << EOF
#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_NAME    "$PACKAGE"
#define PACKAGE_VERSION "$VERSION"
#define PREFIX         "$PREFIX"
#define BINDIR         "$BINDIR"
#define MANDIR         "$MANDIR"

#define BUILD_TYPE     "$BUILD_TYPE"
#define COMPILER       "$CC"

#endif /* CONFIG_H */
EOF

# Create Makefile from template
echo "Creating Makefile..."
sed -e "s|@PREFIX@|$PREFIX|g" \
    -e "s|@BINDIR@|$BINDIR|g" \
    -e "s|@MANDIR@|$MANDIR|g" \
    -e "s|@CC@|$CC|g" \
    -e "s|@CFLAGS@|$CFLAGS|g" \
    -e "s|@LDFLAGS@|$LDFLAGS|g" \
    -e "s|@LIBS@|$LIBS|g" \
    -e "s|@PACKAGE@|$PACKAGE|g" \
    -e "s|@VERSION@|$VERSION|g" \
    Makefile.in > Makefile

# Summary
echo
echo "Configuration summary:"
echo "  Package:      $PACKAGE $VERSION"
echo "  Compiler:     $CC"
echo "  CFLAGS:       $CFLAGS"
echo "  LDFLAGS:      $LDFLAGS"
echo "  LIBS:         $LIBS"
echo "  Build type:   $BUILD_TYPE"
echo
echo "Installation directories:"
echo "  PREFIX:       $PREFIX"
echo "  Binary:       $BINDIR"
echo "  Manual:       $MANDIR"
echo
echo "Configuration completed successfully!"
echo "Run 'make' to build the project"

# Save build timestamp
date "+%Y-%m-%d %H:%M:%S" > .buildtime