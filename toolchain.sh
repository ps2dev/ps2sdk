#!/bin/sh
# toolchain.sh - Dan Peori <peori@oopo.net>
# Copy all you want. Please give me some credit.
#
# ===================
#  ABOUT THIS SCRIPT
# ===================
#
#  This script will automatically download, unpack, patch, build and
#  install a PS2 homebrew development toolchain. It requires a reasonably
#  unix-like environment and access to some simple programs like wget.
#  It will take what seems like a very long time - it does a lot of work.
#
#  Once finished, you need to add the following to your login settings:

  ## PS2DEV SETTINGS
  export PS2DEV="/usr/local/ps2dev"
  export PS2SDK="$PS2DEV/ps2sdk"
  export CVSROOT=":pserver:anonymous@cvs.ps2dev.org:/home/ps2cvs"
  export PATH="$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin"

#
# ===================
#  ABOUT THE PATCHES
# ===================
#
#  binutils-2.14-PS2-20041127.patch:
#   pixel: 20040214 (gruntwork) patch
#   mrbrown: 20040307 patch
#   ryani: 20040214 patch
#   blackd: vu opcode 'w' target fix
#   blackd: 'scei' name fix
#   blackd: 'mfpc' T5 fix
#   pixel/cody56: fix for irx files
#   emoon/pixel/mrbrown: mips-opc.c mfpc fix
#
#  gcc-3.2.2-PS2-20041130.patch:
#   pixel: 20040214 (gruntwork) patch
#   mrbrown: 20040222 patch
#   blackd: 'scei' name fix patch
#   pixel: fixed alignment bug
#   ooPo: bash-3.0 'trap - 0' configure bug
#
#  newlib-1.10.0-PS2-20040312.patch:
#   ps2lib 2.0: patches for fileio
#   ooPo: bash-3.0 'trap - 0' configure bug
#

 ########################
 ## MAIN CONFIGURATION ##
 ########################

  ## Set the source and build directories.
  export SRCDIR="`pwd`"
  export TMPDIR="/tmp/ps2dev"; mkdir -p $TMPDIR

  ## Source code versions.
  export BINUTILS="binutils-2.14"
  export GCC="gcc-3.2.2"
  export NEWLIB="newlib-1.10.0"
  export PS2CLIENT="ps2client-2.0.0"

  ## Patch file versions.
  export BINUTILS_PATCH="binutils-2.14-PS2-20041127"
  export GCC_PATCH="gcc-3.2.2-PS2-20041130"
  export NEWLIB_PATCH="newlib-1.10.0-PS2-20041130"

  ## CVS CONFIGURATION
  if [ "`cat ~/.cvspass | grep $CVSROOT`" = "" ]; then
   echo "THE SECRET PASSWORD IS: anonymous"
   cvs login
  fi

 #######################
 ## SOFTWARE CHECKING ##
 #######################

  ## Check for which make to use.
  export MAKE="gmake"; $MAKE -v || { export MAKE="make"; }

  ## Check for make.
  $MAKE -v || { echo "ERROR: Please make sure you have '$MAKE' installed."; exit; }

  ## Check for patch.
  patch -v || { echo "ERROR: Please make sure you have 'patch' installed."; exit; }

  ## Check for wget.
  wget -V || { echo "ERROR: Please make sure you have 'wget' installed."; exit; }

 ################################
 ## DOWNLOAD, UNPACK AND PATCH ##
 ################################

  ## Download the source.
  wget -c ftp://ftp.gnu.org/pub/gnu/binutils/$BINUTILS.tar.gz
  wget -c ftp://ftp.gnu.org/pub/gnu/gcc/$GCC.tar.gz
  wget -c ftp://sources.redhat.com/pub/newlib/$NEWLIB.tar.gz
  wget -c http://www.oopo.net/consoledev/files/$PS2CLIENT.tar.gz

  ## Download the patches.
  wget -c http://www.oopo.net/consoledev/files/$BINUTILS_PATCH.patch
  wget -c http://www.oopo.net/consoledev/files/$GCC_PATCH.patch
  wget -c http://www.oopo.net/consoledev/files/$NEWLIB_PATCH.patch

 #################################
 ## UNPACK AND PATCH THE SOURCE ##
 #################################

  ## Create the build directory.
  mkdir -p $TMPDIR; cd $TMPDIR

  ## Unpack the source.
  rm -Rf $BINUTILS; tar xfvz $SRCDIR/$BINUTILS.tar.gz
  rm -Rf $GCC; tar xfvz $SRCDIR/$GCC.tar.gz
  rm -Rf $NEWLIB; tar xfvz $SRCDIR/$NEWLIB.tar.gz
  rm -Rf $PS2CLIENT; tar xfvz $SRCDIR/$PS2CLIENT.tar.gz

  ## Patch the source.
  cd $BINUTILS; cat $SRCDIR/$BINUTILS_PATCH.patch | patch -p1; cd ..
  cd $GCC; cat $SRCDIR/$GCC_PATCH.patch | patch -p1; cd ..
  cd $NEWLIB; cat $SRCDIR/$NEWLIB_PATCH.patch | patch -p1; cd ..

 ################################
 ## BUILD AND INSTALL BINUTILS ##
 ################################

  ## Enter the source directory.
  cd $BINUTILS

  ## Build for each target.
  for TARGET in "ee" "iop" "dvp"; do

   ## Create the build directory.
   mkdir build-$TARGET

   ## Enter the build directory.
   cd build-$TARGET

   ## Configure the source.
   ../configure --prefix=$PS2DEV/$TARGET --target=$TARGET || { echo "ERROR CONFIGURING BINUTILS ($BINUTILS $TARGET)"; exit; }

   ## Build the source.
   $MAKE clean; $MAKE || { echo "ERROR BUILDING BINUTILS ($BINUTILS $TARGET)"; exit; }

   ## Install the result.
   $MAKE install || { echo "ERROR INSTALLING BINUTILS ($BINUTILS $TARGET)"; exit; }

   ## Clean up the result.
   $MAKE clean

   ## Exit the build directory.
   cd ..

  ## End of the target build.
  done

  ## Exit the source directory.
  cd ..

 ###########################
 ## BUILD AND INSTALL GCC ##
 ###########################

  ## Enter the source directory.
  cd $GCC

  ## Build for each target.
  for TARGET in "ee" "iop"; do

   ## Create the build directory.
   mkdir build-$TARGET

   ## Enter the build directory.
   cd build-$TARGET

   ## Configure the source.
   ../configure --prefix=$PS2DEV/$TARGET --target=$TARGET --enable-languages="c" --with-newlib --without-headers || { echo "ERROR CONFIGURING GCC ($GCC $TARGET)"; exit; }

   ## Build the source.
   $MAKE clean; $MAKE || { echo "ERROR BUILDING GCC ($GCC $TARGET)"; exit; }

   ## Install the result.
   $MAKE install || { echo "ERROR INSTALLING GCC ($GCC $TARGET)"; exit; }

   ## Clean up the result.
   $MAKE clean

   ## Exit the build directory.
   cd ..

  ## End of the target build.
  done

  ## Exit the source directory.
  cd ..

 ##############################
 ## BUILD AND INSTALL NEWLIB ##
 ##############################

  ## Enter the source directory.
  cd $NEWLIB

  ## Build for each target.
  for TARGET in "ee"; do

   ## Create the build directory.
   mkdir build-$TARGET

   ## Enter the build directory.
   cd build-$TARGET

   ## Configure the source.
   ../configure --prefix=$PS2DEV/$TARGET --target=$TARGET || { echo "ERROR CONFIGURING NEWLIB ($NEWLIB $TARGET)"; exit; }

   ## Build the source.
   $MAKE clean; CPPFLAGS="-G0" $MAKE || { echo "ERROR BUILDING NEWLIB ($NEWLIB $TARGET)"; exit; }

   ## Install the result.
   $MAKE install || { echo "ERROR INSTALLING NEWLIB ($NEWLIB $TARGET)"; exit; }

   ## Clean up the result.
   $MAKE clean

   ## Exit the build directory.
   cd ..

  ## End of the target build.
  done

  ## Exit the source directory.
  cd ..

 #################################
 ## BUILD AND INSTALL GCC (C++) ##
 #################################

  ## Enter the source directory.
  cd $GCC

  ## Build for each target.
  for TARGET in "ee"; do

   ## Create the build directory.
   mkdir build-$TARGET-c++

   ## Enter the build directory.
   cd build-$TARGET-c++

   ## Configure the source.
   ../configure --prefix=$PS2DEV/$TARGET --target=$TARGET --enable-languages="c,c++" --with-newlib --with-headers=$PS2DEV/$TARGET/$TARGET/include --enable-cxx-flags="-G0" || { echo "ERROR CONFIGURING GCC ($GCC $TARGET C++)"; exit; }

   ## Build the source.
   $MAKE clean; $MAKE CFLAGS_FOR_TARGET="-G0" || { echo "ERROR BUILDING GCC ($GCC $TARGET C++)"; exit; }

   ## Install the result.
   $MAKE install || { echo "ERROR INSTALLING GCC ($GCC $TARGET C++)"; exit; }

   ## Clean up the result.
   $MAKE clean

   ## Exit the build directory.
   cd ..

  ## End of the target build.
  done

  ## Exit the source directory.
  cd ..

 #################################
 ## BUILD AND INSTALL PS2CLIENT ##
 #################################

  ## Enter the source directory.
  cd $PS2CLIENT

  ## Build the source.
  $MAKE clean; $MAKE || { echo "ERROR BUILDING PS2CLIENT"; exit; }

  ## Install the result.
  $MAKE install || { echo "ERROR INSTALLING PS2CLIENT"; exit; }

  ## Clean up the result.
  $MAKE clean

  ## Exit the source directory.
  cd ..

 ##############################
 ## BUILD AND INSTALL PS2SDK ##
 ##############################

  ## Remove any previous builds.
  rm -Rf ps2sdk

  ## Check out the latest source.
  cvs checkout ps2sdk

  ## Enter the source directory.
  cd ps2sdk

  ## Configure the source.
  export PS2SDKSRC="`pwd`"

  ## Build the source.
  $MAKE clean; $MAKE || { echo "ERROR BUILDING PS2SDK"; exit; }

  ## Install the result.
  $MAKE release || { echo "ERROR INSTALLING PS2SDK"; exit; }
  
  ## Replace newlib's crt0 with the one in ps2sdk.
  cp $(PS2SDK)/ee/startup/crt0.o $(PS2DEV)/ee/lib/gcc-lib/ee/3.2.2/
  cp $(PS2SDK)/ee/startup/crt0.o $(PS2DEV)/ee/ee/lib/

  ## Clean up the result.
  $MAKE clean

  ## Exit the source directory.
  cd ..

 #########################
 ## CLEAN UP THE RESULT ##
 #########################

  ## Clean up binutils.
  rm -Rf $BINUTILS

  ## Clean up gcc.
  rm -Rf $GCC

  ## Clean up newlib.
  rm -Rf $NEWLIB

  ## Clean up ps2client.
  rm -Rf $PS2CLIENT

  ## Clean up ps2sdk.
  rm -Rf ps2sdk
