#!/bin/sh

if [ "$#" -ne 3 ]; then
    echo "Syntax: $0 <path to bin directory> <qt prefix> <output directory>"
    exit 1
fi

BINDIR=$1
QTDIR=$2
OUTDIR=$3

BINARY=aethersx2
QTLIBS="libQt6Core.so.6 libQt6DBus.so.6 libQt6Gui.so.6 libQt6Network.so.6 libQt6OpenGL.so.6 libQt6WaylandClient.so.6 libQt6WaylandCompositor.so.6 libQt6WaylandEglClientHwIntegration.so.6 libQt6WaylandEglCompositorHwIntegration.so.6 libQt6Widgets.so.6"
QTPLUGINS="plugins/iconengines plugins/imageformats plugins/platforms plugins/platformthemes plugins/wayland-decoration-client plugins/wayland-graphics-integration-client plugins/wayland-graphics-integration-server plugins/wayland-shell-integration"

set -e

mkdir -p $OUTDIR

echo "Copying binary and resources..."
cp -av $BINDIR/$BINARY $BINDIR/resources $BINDIR/shaders $OUTDIR/

echo "Patching RPATH in ${BINARY}..."
patchelf --set-rpath '$ORIGIN' $OUTDIR/$BINARY
llvm-strip $OUTDIR/$BINARY

echo "Copying Qt libraries..."

for lib in $QTLIBS; do
	cp -avL $QTDIR/lib/$lib $OUTDIR
done

echo "Copying Qt plugins..."

mkdir -p $OUTDIR/plugins
for plugin in $QTPLUGINS; do
	mkdir -p $OUTDIR/$plugin
	cp -avL $QTDIR/$plugin/*.so $OUTDIR/$plugin/
done

for so in $(find $OUTDIR/plugins -iname '*.so'); do
	echo "Patching RPATH in ${so}..."
	patchelf --set-rpath '$ORIGIN/../..' $so
done

echo "Creating qt.conf..."
cat > $OUTDIR/qt.conf << EOF
[Paths]
Plugins = ./plugins
EOF


