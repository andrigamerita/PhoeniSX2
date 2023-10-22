#!/usr/bin/env bash

if [ "$#" -ne 3 ]; then
    echo "Syntax: $0 <path to bin directory> <qt prefix> <chroot directory>"
    exit 1
fi

BINDIR=$1
QTDIR=$2
CHROOT=$3

BINARY=aethersx2
SYSLIBS="libaio.so.1 libapparmor.so.1 libblkid.so.1 libbsd.so.0 libdbus-1.so.3 libgcrypt.so.20 liblzma.so.5 libmount.so.1 libnsl.so.1 libpcre.so.3 libselinux.so.1 libsystemd.so.0 libudev.so.1 libwrap.so.0 libFLAC.so.8 libSDL2-2.0.so.0 libSoundTouch.so.1 libXau.so.6 libXcomposite.so.1 libXcursor.so.1 libXdamage.so.1 libXdmcp.so.6 libXext.so.6 libXfixes.so.3 libXi.so.6 libXinerama.so.1 libXrandr.so.2 libXrender.so.1 libXxf86vm.so.1 libasyncns.so.0 libcrypto.so.1.1 libjpeg.so.8 liblz4.so.1 libogg.so.0 libpcap.so.0.8 libpng16.so.16 libpulse.so.0 libsamplerate.so.0 libsndfile.so.1 libvorbis.so.0 libvorbisenc.so.2 libwayland-client.so.0 libwayland-cursor.so.0 libwayland-egl.so.1 libwx_baseu-3.0.so.0 libxcb-render.so.0 libxcb-shm.so.0 libxkbcommon.so.0 pulseaudio/libpulsecommon-13.99.so"
QTLIBS="libQt6Core.so.6 libQt6DBus.so.6 libQt6Gui.so.6 libQt6Network.so.6 libQt6OpenGL.so.6 libQt6WaylandClient.so.6 libQt6WaylandCompositor.so.6 libQt6WaylandEglClientHwIntegration.so.6 libQt6WaylandEglCompositorHwIntegration.so.6 libQt6Widgets.so.6"
QTPLUGINS="plugins/iconengines plugins/imageformats plugins/platforms plugins/platformthemes plugins/wayland-decoration-client plugins/wayland-graphics-integration-client plugins/wayland-graphics-integration-server plugins/wayland-shell-integration"

set -e

if [ ! -f AppRun-aarch64 ]; then
	wget https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-aarch64
fi
if [ ! -f appimagetool-x86_64.AppImage ]; then
	wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
	chmod +x appimagetool-x86_64.AppImage
fi

rm -fr AetherSX2.AppDir
mkdir AetherSX2.AppDir
OUTDIR=$(realpath ./AetherSX2.AppDir)
SCRIPTDIR=$(dirname ${BASH_SOURCE[0]})

mkdir -p $OUTDIR/usr/bin $OUTDIR/usr/lib $OUTDIR/usr/lib/pulseaudio

echo "Copying binary and resources..."
cp -av $BINDIR/$BINARY $BINDIR/resources $BINDIR/shaders $OUTDIR/usr/bin

echo "Patching RPATH in ${BINARY}..."
patchelf --set-rpath '$ORIGIN/../lib' $OUTDIR/usr/bin/$BINARY
llvm-strip $OUTDIR/usr/bin/$BINARY

echo "Copying system libraries..."
for lib in $SYSLIBS; do
	echo "$CHROOT/lib/aarch64-linux-gnu/$lib"
	if [ -f "$CHROOT/lib/aarch64-linux-gnu/$lib" ]; then
		cp "$CHROOT/lib/aarch64-linux-gnu/$lib" "$OUTDIR/usr/lib/$lib"
	elif [ -f "$CHROOT/usr/lib/aarch64-linux-gnu/$lib" ]; then
		cp "$CHROOT/usr/lib/aarch64-linux-gnu/$lib" "$OUTDIR/usr/lib/$lib"
	elif [ -f "$CHROOT/lib/$lib" ]; then
		cp "$CHROOT/lib/$lib" "$OUTDIR/usr/lib/$lib"
	elif [ -f "$CHROOT/usr/lib/$lib" ]; then
		cp "$CHROOT/usr/lib/$lib" "$OUTDIR/usr/lib/$lib"
	else
		echo "*** Failed to find '$lib'"
		exit 1
	fi

	llvm-strip $OUTDIR/usr/lib/$lib
done

echo "Copying Qt libraries..."

for lib in $QTLIBS; do
	cp -avL $QTDIR/lib/$lib $OUTDIR/usr/lib
done

echo "Copying Qt plugins..."

mkdir -p $OUTDIR/usr/lib/plugins
for plugin in $QTPLUGINS; do
	mkdir -p $OUTDIR/usr/lib/$plugin
	cp -avL $QTDIR/$plugin/*.so $OUTDIR/usr/lib/$plugin/
	for pluginfile in $(find $OUTDIR/usr/lib/$plugin -name '*.so'); do
		llvm-strip $pluginfile
	done
done

for so in $(find $OUTDIR/usr/lib/plugins -iname '*.so'); do
	echo "Patching RPATH in ${so}..."
	patchelf --set-rpath '$ORIGIN/../..' $so
done

echo "Creating qt.conf..."
cat > $OUTDIR/usr/bin/qt.conf << EOF
[Paths]
Plugins = ../lib/plugins
EOF

echo "Copy desktop/icon..."
cp -v $SCRIPTDIR/aethersx2.desktop $OUTDIR/
cp -v $SCRIPTDIR/aethersx2.png $OUTDIR/
cp -v $SCRIPTDIR/AppRun $OUTDIR/

echo "Generate AppImage"
./appimagetool-x86_64.AppImage -v --runtime-file $SCRIPTDIR/runtime-aarch64 AetherSX2.AppDir AetherSX2-aarch64.AppImage

