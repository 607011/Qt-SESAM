@echo off
cd %1
for %%F in ("*.wmv") do (
  ffmpeg -y -i "%%F" -an -preset veryslow -qp 0 -movflags +faststart -vcodec libx264 output.mp4
  ffmpeg -y -i "%%F" -an -qscale 0 -preset veryslow -vcodec libvpx output.webm
)
ffmpeg -y -ss 100 -i output.mp4 -f mjpeg -vframes 1 output.jpg
cd %~dp0
