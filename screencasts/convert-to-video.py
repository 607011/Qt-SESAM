#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys, subprocess
from os import chdir
from glob import glob

# FFMPEG = 'D:\\Developer\\ffmpeg.exe'
FFMPEG = 'D:\\Developer\\ImageMagick-6.8.6-Q16\\ffmpeg.exe'
FFPROBE = 'D:\\Developer\\ffprobe.exe'

def main():
    folder = sys.argv[1]
    chdir(folder)
    f = glob('*.wmv')[0]
    subprocess.call([FFMPEG, '-y', '-i', f, '-an', '-q:v', '3', '-preset', 'veryslow', '-vcodec', 'libvpx', '-vf', 'fps=10', 'output.webm'])
    subprocess.call([FFMPEG, '-y', '-i', f, '-an', '-q:v', '7', '-preset', 'veryslow', '-vcodec', 'libtheora', '-vf', 'fps=10', 'output.ogv'])
    subprocess.call([FFMPEG, '-y', '-i', f, '-an', '-q:v', '4', '-preset', 'veryslow', '-vcodec', 'libx264', '-vf', 'fps=10', '-movflags', '+faststart', 'output.mp4'])
    p = subprocess.Popen([FFPROBE, '-of', 'flat=s=_', '-v', 'error', '-show_entries', 'format=duration', 'output.mp4'], stdout=subprocess.PIPE)
    p.wait()
    duration = str(0.75 * float(p.communicate()[0].split('=')[1].rstrip()[1:-1]))
    subprocess.call([FFMPEG, '-y', '-i', 'output.mp4', '-vframes', '1', '-ss', duration, 'output.png'])
    chdir('..')


if __name__ == '__main__':
    main()