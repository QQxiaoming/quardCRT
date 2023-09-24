# the convert command comes from imagemagick
for size in 16 32 64 128 256; do
  half="$(($size / 2))"
  convert ico.png -resize x$size $1/icon_${size}x${size}.png
  convert ico.png -resize x$size $1/icon_${half}x${half}@2x.png
done

iconutil -c icns $1
